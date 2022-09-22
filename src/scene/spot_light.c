#include "spot_light.h"

#include <math.h>

#include "../math/plane.h"
#include "../defs.h"

struct Vector3 gLightCircle[LIGHT_CIRCLE_POINT_COUNT] = {
    {1.0f, 0.0f, -1.0f},
    {0.707f, 0.707f, -1.0f},
    {0.0f, 1.0f, -1.0f},
    {-0.707f, 0.707f, -1.0f},
    {-1.0f, 0.0f, -1.0f},
    {-0.707f, -0.707f, -1.0f},
    {0.0f, -1.0f, -1.0f},
    {0.707f, -0.707f, -1.0f},
};

struct Vector3 gLightCenterDir = {0.0f, 0.0f, -1.0f};

void spotLightSetAngle(struct SpotLight* spotLight, float angle) {
    float halfAngle = angle * 0.5f;
    spotLight->horizontalScale = tanf(halfAngle);
}

void spotLightUpdateGeometry(struct SpotLight* spotLight, struct Vector3* cameraPos) {
    struct Vector3 origin = spotLight->transform.position;

    struct Vector3 lightDirection[LIGHT_CIRCLE_POINT_COUNT];

    spotLight->boundingBox.min = origin;
    spotLight->boundingBox.max = origin;

    int isBackFaceMask = 0;

    for (unsigned i = 0; i < LIGHT_CIRCLE_POINT_COUNT; ++i) {
        struct Vector3 rayDirection;
        rayDirection.x = gLightCircle[i].x * spotLight->horizontalScale;
        rayDirection.y = gLightCircle[i].y * spotLight->horizontalScale;
        rayDirection.z = gLightCircle[i].z;

        vector3Normalize(&rayDirection, &rayDirection);
        quatMultVector(&spotLight->transform.rotation, &rayDirection, &rayDirection);

        if (rayDirection.y > -0.1f) {
            return;
        }

        float distance = -origin.y / rayDirection.y;

        vector3AddScaled(&origin, &rayDirection, distance, &spotLight->lightOutline[i]);
        lightDirection[i] = rayDirection;

        vector3Max(&spotLight->boundingBox.max, &spotLight->lightOutline[i], &spotLight->boundingBox.max);
        vector3Min(&spotLight->boundingBox.min, &spotLight->lightOutline[i], &spotLight->boundingBox.min);
    }

    struct Vector3 dirFromCamera;

    vector3Sub(&origin, cameraPos, &dirFromCamera);

    for (unsigned i = 0; i < LIGHT_CIRCLE_POINT_COUNT; ++i) {
        vector3Cross(&lightDirection[(i + 1) % LIGHT_CIRCLE_POINT_COUNT], &lightDirection[i], &spotLight->faceNormal[i]);
        vector3Normalize(&spotLight->faceNormal[i], &spotLight->faceNormal[i]);

        if (vector3Dot(&dirFromCamera, &spotLight->faceNormal[i]) > 0.0f) {
            isBackFaceMask |= (1 << i);
        }
    }

    spotLight->isBackFaceMask = isBackFaceMask;

    quatMultVector(&spotLight->transform.rotation, &gLightCenterDir, &spotLight->centerDirection);
    spotLight->borderDot = vector3Dot(&spotLight->centerDirection, &lightDirection[0]);

    spotLight->boundingBox.min.y -= 0.5f;
}

void spotLightInit(struct SpotLight* spotLight, struct SpotLightDefinition* spotLightDef, struct Vector3* cameraPos) {
    spotLight->transform.position = spotLightDef->position;
    spotLight->transform.rotation = spotLightDef->rotation;
    spotLight->transform.scale = gOneVec;

    spotLightSetAngle(spotLight, spotLightDef->angle);

    spotLightUpdateGeometry(spotLight, cameraPos);
}

void spotLightUpdate(struct SpotLight* spotLight, struct Vector3* cameraPos) {
    spotLightUpdateGeometry(spotLight, cameraPos);
}

void spotLightRenderProjection(struct SpotLight* spotLight, struct RenderState* renderState) {
    Vtx* vertices = renderStateRequestVertices(renderState, 8);

    for (unsigned i = 0; i < LIGHT_CIRCLE_POINT_COUNT; ++i) {
        vertices[i].v.ob[0] = (short)(SCENE_SCALE * spotLight->lightOutline[i].x);
        vertices[i].v.ob[1] = (short)(SCENE_SCALE * spotLight->lightOutline[i].y);
        vertices[i].v.ob[2] = (short)(SCENE_SCALE * spotLight->lightOutline[i].z);

        vertices[i].v.flag = 0;

        vertices[i].v.tc[0] = 0;
        vertices[i].v.tc[1] = 0;

        vertices[i].v.cn[0] = 0;
        vertices[i].v.cn[1] = 0;
        vertices[i].v.cn[2] = 0;
        vertices[i].v.cn[3] = 255;
    }

    Gfx* displayList = renderStateAllocateDLChunk(renderState, 5);

    Gfx* dl = displayList;

    gSPVertex(dl++, vertices, LIGHT_CIRCLE_POINT_COUNT, 0);
    gSP2Triangles(dl++, 0, 1, 2, 0, 0, 2, 3, 0);
    gSP2Triangles(dl++, 0, 3, 4, 0, 0, 4, 5, 0);
    gSP2Triangles(dl++, 0, 5, 6, 0, 0, 6, 7, 0);
    gSPEndDisplayList(dl++);

    gSPDisplayList(renderState->dl++, displayList);
}

struct SpotLightFace {
    struct SpotLight* spotLight;
    int faceIndex;
};

int spotLightFaceSum(void* data, struct Vector3* direction, struct Vector3* output) {
    struct SpotLightFace* face = (struct SpotLightFace*)data;

    int result = 0;
    float dotCompare = vector3Dot(&face->spotLight->transform.position, direction);

    struct Vector3* firstVertex = &face->spotLight->lightOutline[face->faceIndex];
    float test = vector3Dot(firstVertex, direction);

    if (test > dotCompare) {
        dotCompare = test;
        result = 1;
    }

    struct Vector3* secondVertex = &face->spotLight->lightOutline[(face->faceIndex + 1) % LIGHT_CIRCLE_POINT_COUNT];
    test = vector3Dot(secondVertex, direction);

    if (test > dotCompare) {
        dotCompare = test;
        result = 2;
    }
    
    switch (result) {
        case 0: 
            *output = face->spotLight->transform.position;
            break;
        case 1:
            *output = *firstVertex;
            break;
        case 2:
            *output = *secondVertex;
            break;
    }

    return 1 << result;
}

float spotLightClosenessWeight(struct SpotLight* spotLight, struct Vector3* point) {
    struct Vector3 offset;
    vector3Sub(point, &spotLight->transform.position, &offset);
    vector3Normalize(&offset, &offset);

    float weight = vector3Dot(&offset, &spotLight->centerDirection);

    if (weight < spotLight->borderDot) {
        return 0.0f;
    }

    weight -= spotLight->borderDot;
    weight /= (1.0f - spotLight->borderDot);

    return weight;
}

enum LightIntersection spotLightPointIsInside(struct SpotLight* spotLight, struct Vector3* point) {
    if (!box3DContainsPoint(&spotLight->boundingBox, point)) {
        return LightIntersectionOutside;
    }

    struct Vector3 offset;

    vector3Sub(&spotLight->transform.position, point, &offset);

    for (int i = 0; i < LIGHT_CIRCLE_POINT_COUNT; ++i) {
        if (vector3Dot(&offset, &spotLight->faceNormal[i]) > 0.0f) {
            return LightIntersectionOutside;
        }
    }

    return LightIntersectionInside;
}


enum LightIntersection spotLightIsInside(struct SpotLight* spotLight, struct CollisionObject* collisionObject) {
    if (!box3DHasOverlap(&spotLight->boundingBox, &collisionObject->boundingBox)) {
        return LightIntersectionOutside;
    }

    int isInside = 1;
    int hasBackFaceCollision = 0;
    int hasFrontFaceCollision = 0;

    struct Simplex simplex;

    for (int i = 0; i < LIGHT_CIRCLE_POINT_COUNT; ++i) {
        if (gjkCheckForOverlap(
            &simplex, 
            NULL, 
            spotLightFaceSum, 
            COLLISION_OBJECT_DATA_POINTER(collisionObject), 
            collisionObject->minkowskiSum, 
            &spotLight->faceNormal[i]
        )) {
            hasBackFaceCollision = 1;
            hasFrontFaceCollision = 1;
        }
    }

    if (isInside) {
        return LightIntersectionInside;
    }

    return (hasBackFaceCollision ? LightIntersectionTouchingBackFace : 0) | (hasFrontFaceCollision ? LightIntersectionTouchingFrontFace : 0);
}

int spotLightsFindConfiguration(struct SpotLight* lights, int lightCount, struct Vector3* point, struct CollisionObject* collisionObject, struct LightConfiguration* output) {
    float primaryLightNearness = -1.0f;
    float secondaryLightNearness = -1.0f;

    output->primaryLight = NULL;
    output->secondaryLight = NULL;
    output->blendWeight = 0.0f;

    for (int i = 0; i < lightCount; ++i) {
        if (!spotLightPointIsInside(&lights[i], point)) {
            continue;
        }

        float weight = spotLightClosenessWeight(&lights[i], point);

        if (weight > primaryLightNearness) {
            secondaryLightNearness = primaryLightNearness;
            output->secondaryLight = output->primaryLight;

            primaryLightNearness = weight;
            output->primaryLight = &lights[i];
        } else if (weight > secondaryLightNearness) {
            secondaryLightNearness = weight;
            output->secondaryLight = &lights[i];
        }
    }

    if (!output->primaryLight) {
        return 0;
    }


    if (!output->secondaryLight) {
        output->blendWeight = 0.0f;
        return 1;
    }

    float totalWeight = primaryLightNearness + secondaryLightNearness;

    output->blendWeight = secondaryLightNearness / totalWeight;

    return 2;
}

Light* spotLightsSetupLight(struct LightConfiguration* lightConfig, struct Vector3* target, struct RenderState* renderState) {
    struct Vector3 lightSource;

    if (!spotLightsGetPosition(lightConfig, &lightSource)) {
        Light* light = renderStateRequestLights(renderState, 1);
        light->l.col[0] = 0;
        light->l.col[1] = 0;
        light->l.col[2] = 0;
        light->l.pad1 = 0;
        light->l.colc[0] = 0;
        light->l.colc[1] = 0;
        light->l.colc[2] = 0;
        light->l.pad1 = 0;
        vector3ToVector3u8(&gRight, (struct Vector3u8*)light->l.dir);
        light->l.pad3 = 0;
        return light;
    }

    struct Vector3 offset;
    vector3Sub(&lightSource, target, &offset);
    vector3Normalize(&offset, &offset);

    Light* light = renderStateRequestLights(renderState, 1);
    light->l.col[0] = 255;
    light->l.col[1] = 255;
    light->l.col[2] = 255;
    light->l.pad1 = 0;
    light->l.colc[0] = 255;
    light->l.colc[1] = 255;
    light->l.colc[2] = 255;
    light->l.pad1 = 0;
    vector3ToVector3u8(&offset, (struct Vector3u8*)light->l.dir);
    light->l.pad3 = 0;
    return light;
}

int spotLightsGetPosition(struct LightConfiguration* lightConfig, struct Vector3* position) {
    if (!lightConfig->primaryLight) {
        return 0;
    }

    if (lightConfig->secondaryLight) {
        vector3Lerp(
            &lightConfig->primaryLight->transform.position, 
            &lightConfig->secondaryLight->transform.position, 
            lightConfig->blendWeight,
            position
        );
    } else {
        *position = lightConfig->primaryLight->transform.position;
    }

    return 1;
}
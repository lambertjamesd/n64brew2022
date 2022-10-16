#include "spot_light.h"

#include <math.h>

#include "../math/plane.h"
#include "../defs.h"
#include "../util/time.h"

#include "../build/assets/models/lantern.h"
#include "../build/assets/models/table.h"
#include "../build/assets/materials/static.h"

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

struct Vector3 gRelativeAnchorPoint = {0.0f, 0.0f, 0.5};

struct Vector3 gLightCenterDir = {0.0f, 0.0f, -1.0f};

#define MASS                1.0f
#define SPRING_CONSTANT     0.1f
#define DAMPING             0.999f
#define ANCHOR_OFFSET       3.2f

#define WIND_SPEED          -2.0f
#define WIND_PERIOD         10.0f
#define WIND_STRENGTH       0.01f
#define WIND_THRESHOLD      0.2f

void spotLightSetAngle(struct SpotLight* spotLight, float angle) {
    float halfAngle = angle * 0.5f;
    spotLight->horizontalScale = tanf(halfAngle);
}

void spotLightUpdateGeometry(struct SpotLight* spotLight, struct Vector3* cameraPos) {
    rigidBodyUpdate(&spotLight->rigidBody);

    struct Vector3 anchorPoint;
    transformPoint(&spotLight->rigidBody.transform, &gRelativeAnchorPoint, &anchorPoint);

    struct Vector3 offset;
    vector3Sub(&spotLight->worldAnchorPoint, &anchorPoint, &offset);
    vector3Scale(&offset, &offset, SPRING_CONSTANT);

    rigidBodyAppyImpulse(&spotLight->rigidBody, &anchorPoint, &offset);

    float strength = sinf(gTimePassed * (M_PI * 2.0f / WIND_PERIOD) + spotLight->rigidBody.transform.position.x * (2.0f * M_PI / WIND_SPEED));

    if (strength > WIND_THRESHOLD) {
        vector3Scale(&gRight, &offset, strength * WIND_STRENGTH);
        rigidBodyAppyImpulse(&spotLight->rigidBody, &spotLight->rigidBody.transform.position, &offset);
    }

    vector3Scale(&spotLight->rigidBody.velocity, &spotLight->rigidBody.velocity, DAMPING);
    vector3Scale(&spotLight->rigidBody.angularVelocity, &spotLight->rigidBody.angularVelocity, DAMPING);

    struct Vector3 origin = spotLight->rigidBody.transform.position;

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
        quatMultVector(&spotLight->rigidBody.transform.rotation, &rayDirection, &rayDirection);

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

        if (vector3Dot(&dirFromCamera, &spotLight->faceNormal[i]) < 0.0f) {
            isBackFaceMask |= (1 << i);
        }
    }

    spotLight->isBackFaceMask = isBackFaceMask;

    quatMultVector(&spotLight->rigidBody.transform.rotation, &gLightCenterDir, &spotLight->centerDirection);
    spotLight->borderDot = vector3Dot(&spotLight->centerDirection, &lightDirection[0]);

    spotLight->boundingBox.min.y -= 0.5f;
}

void spotLightInit(struct SpotLight* spotLight, struct SpotLightDefinition* spotLightDef, struct Vector3* cameraPos) {
    rigidBodyInit(&spotLight->rigidBody, MASS, 1.0f);
    spotLight->rigidBody.transform.position = spotLightDef->position;
    spotLight->rigidBody.transform.rotation = spotLightDef->rotation;

    spotLightSetAngle(spotLight, spotLightDef->angle);

    transformPoint(&spotLight->rigidBody.transform, &gRelativeAnchorPoint, &spotLight->worldAnchorPoint);
    spotLight->worldAnchorPoint.y += ANCHOR_OFFSET;

    spotLightUpdateGeometry(spotLight, cameraPos);
}

void spotLightUpdate(struct SpotLight* spotLight, struct Vector3* cameraPos) {
    spotLightUpdateGeometry(spotLight, cameraPos);
}

void spotLightRender(struct SpotLight* spotLight, struct RenderScene* renderScene) {
    Mtx* mtx = renderStateRequestMatrices(renderScene->renderState, 1);

    transformToMatrixL(&spotLight->rigidBody.transform, mtx, SCENE_SCALE);

    renderSceneAdd(renderScene, lantern_model_gfx, mtx, LIGHTS_WAREHOUSE_INDEX, &spotLight->rigidBody.transform.position, NULL, NULL);
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

float spotLightShadowSortOrder(struct SpotLight* spotLight, int index) {
    float result = spotLight->lightOutline[index].z;

    int nextIndex = index == (LIGHT_CIRCLE_POINT_COUNT - 1) ? 0 : index + 1;

    if (spotLight->lightOutline[nextIndex].z < result) {
        result = spotLight->lightOutline[nextIndex].z;
    }

    return result;
}

float spotLightClosenessWeight(struct SpotLight* spotLight, struct Vector3* point) {
    struct Vector3 offset;
    vector3Sub(point, &spotLight->rigidBody.transform.position, &offset);
    vector3Normalize(&offset, &offset);

    float weight = vector3Dot(&offset, &spotLight->centerDirection);

    if (weight < spotLight->borderDot) {
        return 0.0f;
    }

    weight -= spotLight->borderDot;
    weight /= (1.0f - spotLight->borderDot);

    return weight;
}

float spotLightMeasureDepth(struct SpotLight* spotLight, struct Vector3* point, float radius) {
    struct Box3D extendedBox;

    box3DExtend(&spotLight->boundingBox, radius, &extendedBox);

    if (!box3DContainsPoint(&extendedBox, point)) {
        return -1.0f;
    }

    struct Vector3 offset;

    vector3Sub(&spotLight->rigidBody.transform.position, point, &offset);

    float minDistnace = 1000.0f;

    for (int i = 0; i < LIGHT_CIRCLE_POINT_COUNT; ++i) {
        float distance = radius - vector3Dot(&offset, &spotLight->faceNormal[i]);

        if (distance < 0.0f) {
            return -1.0;
        }

        if (distance < minDistnace) {
            minDistnace = distance;
        }
    }

    return minDistnace;
}

void vectorToVtx(struct Vector3* vector, Vtx* output) {
    output->v.ob[0] = (SCENE_SCALE * vector->x);
    output->v.ob[1] = (SCENE_SCALE * vector->y);
    output->v.ob[2] = (SCENE_SCALE * vector->z);

    output->v.flag = 0;

    output->v.tc[0] = 0;
    output->v.tc[1] = 0;

    output->v.cn[0] = 0;
    output->v.cn[1] = 0;
    output->v.cn[2] = 0;
    output->v.cn[3] = 255;
}

Gfx* spotLightShadowPlane(struct SpotLight* spotLight, int index, struct RenderState* renderState) {
    Gfx* result = renderStateAllocateDLChunk(renderState, 3);

    Vtx* vertices = renderStateRequestVertices(renderState, 3);

    vectorToVtx(&spotLight->rigidBody.transform.position, &vertices[0]);
    vectorToVtx(&spotLight->lightOutline[index], &vertices[1]);
    vectorToVtx(&spotLight->lightOutline[(index + 1) & (LIGHT_CIRCLE_POINT_COUNT - 1)], &vertices[2]);

    Gfx* dl = result;

    gSPVertex(dl++, vertices, 3, 0);
    gSP1Triangle(dl++, 0, 1, 2, 0);
    gSPEndDisplayList(dl++);

    return result;
}

int spotLightsFindConfiguration(struct SpotLight* lights, int lightCount, struct Vector3* point, float pointRadius, struct LightConfiguration* output) {
    float primaryLightNearness = -1.0f;
    float secondaryLightNearness = -1.0f;

    output->primaryLight = NULL;
    output->secondaryLight = NULL;
    output->blendWeight = 0.0f;

    for (int i = 0; i < lightCount; ++i) {
        float weight = spotLightMeasureDepth(&lights[i], point, pointRadius);

        if (weight <= 0.0f) {
            continue;
        }

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
            &lightConfig->primaryLight->rigidBody.transform.position, 
            &lightConfig->secondaryLight->rigidBody.transform.position, 
            lightConfig->blendWeight,
            position
        );
    } else {
        *position = lightConfig->primaryLight->rigidBody.transform.position;
    }

    return 1;
}
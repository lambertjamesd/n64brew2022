#include "spot_light.h"

#include <math.h>

#include "../math/plane.h"
#include "../defs.h"

#define LIGHT_CIRCLE_POINT_COUNT    8

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

void spotLightSetAngle(struct SpotLight* spotLight, float angle) {
    float halfAngle = angle * 0.5f;
    spotLight->horizontalScale = tanf(halfAngle);
}

void spotLightInit(struct SpotLight* spotLight, struct SpotLightDefinition* spotLightDef) {
    spotLight->transform.position = spotLightDef->position;
    spotLight->transform.rotation = spotLightDef->rotation;
    spotLight->transform.scale = gOneVec;

    spotLightSetAngle(spotLight, spotLightDef->angle);
}

void spotLightRenderProjection(struct SpotLight* spotLight, struct RenderState* renderState) {
    Vtx* vertices = renderStateRequestVertices(renderState, 8);

    struct Vector3 origin = spotLight->transform.position;

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

        struct Vector3 groundIntersection;
        vector3AddScaled(&origin, &rayDirection, distance, &groundIntersection);

        vertices[i].v.ob[0] = (short)(SCENE_SCALE * groundIntersection.x);
        vertices[i].v.ob[1] = (short)(SCENE_SCALE * groundIntersection.y);
        vertices[i].v.ob[2] = (short)(SCENE_SCALE * groundIntersection.z);

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
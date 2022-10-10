#include "table_surface.h"

#include "../defs.h"

#define MAX_LOOP_SIZE   16

void tableSurfaceRenderLight(struct TableSurfaceMesh* surface, struct Vector3* offset, struct SpotLight* spotLight, struct RenderState* renderState) {
    struct Vector3 pointLoop[MAX_LOOP_SIZE];
    struct Vector3 pointLoopTmp[MAX_LOOP_SIZE];

    for (int i = 0; i < surface->vertexCount; ++i) {
        vector3Add(&surface->vertices[i], offset, &pointLoop[i]);
        vector3Sub(&pointLoop[i], &spotLight->rigidBody.transform.position, &pointLoop[i]);
    }

    struct Vector3* src = pointLoop;
    struct Vector3* target = pointLoopTmp;

    int sourceCount = surface->vertexCount;

    for (int faceIndex = 0; faceIndex < LIGHT_CIRCLE_POINT_COUNT; ++faceIndex) {
        if (sourceCount == 0) {
            return;
        }

        float firstDot = vector3Dot(&spotLight->faceNormal[faceIndex], &src[0]);
        int destCount = 0;
        
        if (firstDot > 0.0f) {
            target[destCount] = src[0];
            ++destCount;
        }

        float prevDot = firstDot;

        for (int pointIndex = 1; pointIndex < sourceCount; ++pointIndex) {
            float currentDot = vector3Dot(&spotLight->faceNormal[faceIndex], &src[pointIndex]);

            if ((prevDot > 0.0f && currentDot < 0.0f) || (prevDot < 0.0f && currentDot > 0.0f)) {
                float lerp = prevDot / (prevDot - currentDot);
                vector3Lerp(&src[pointIndex - 1], &src[pointIndex], lerp, &target[destCount]);
                ++destCount;

                if (destCount == MAX_LOOP_SIZE) {
                    break;
                }
            }

            if (currentDot > 0.0f) {
                target[destCount] = src[pointIndex];
                ++destCount;

                if (destCount == MAX_LOOP_SIZE) {
                    break;
                }
            }

            prevDot = currentDot;
        }

        if ((prevDot > 0.0f && firstDot < 0.0f) || (prevDot < 0.0f && firstDot > 0.0f)) {
            float lerp = prevDot / (prevDot - firstDot);
            vector3Lerp(&src[sourceCount - 1], &src[0], lerp, &target[destCount]);
            ++destCount;
        }

        sourceCount = destCount;
        struct Vector3* tmp = src;
        src = target;
        target = tmp;
    }

    if (sourceCount == 0) {
        return;
    }

    Vtx* vertices = renderStateRequestVertices(renderState, sourceCount);

    for (int i = 0; i < sourceCount; ++i) {
        Vtx* curr = &vertices[i];

        curr->v.ob[0] = (short)((spotLight->rigidBody.transform.position.x + src[i].x) * SCENE_SCALE);
        curr->v.ob[1] = (short)((spotLight->rigidBody.transform.position.y + src[i].y) * SCENE_SCALE);
        curr->v.ob[2] = (short)((spotLight->rigidBody.transform.position.z + src[i].z) * SCENE_SCALE);

        curr->v.flag = 0;

        curr->v.tc[0] = 0;
        curr->v.tc[1] = 0;

        curr->v.cn[0] = 255;
        curr->v.cn[1] = 255;
        curr->v.cn[2] = 255;
        curr->v.cn[3] = 255;
    }

    gSPVertex(renderState->dl++, vertices, sourceCount, 0);
    
    for (int i = 2; i < sourceCount; i += 2) {
        if (i + 1 < sourceCount) {
            gSP2Triangles(renderState->dl++, 0, i - 1, i, 0, 0, i, i + 1, 0);
        } else {
            gSP1Triangle(renderState->dl++, 0, i - 1, i, 0);
        }
    }
}

void tableSurfaceRenderShadow(struct TableSurfaceMesh* surface, struct Vector3* offset, struct Vector3* spotLightPos, struct RenderState* renderState) {
    int vertexCount = surface->vertexCount * 2;

    struct Vector3 verticesAsFloat[vertexCount];

    float lightHeight = -spotLightPos->y;

    for (int i = 0; i < surface->vertexCount; ++i) {
        vector3Add(&surface->vertices[i], offset, &verticesAsFloat[i]);

        struct Vector3 rayDirection;
        vector3Sub(&verticesAsFloat[i], spotLightPos, &rayDirection);

        vector3Scale(&rayDirection, &rayDirection, lightHeight / rayDirection.y);

        vector3Add(&rayDirection, spotLightPos, &verticesAsFloat[i + surface->vertexCount]);
        verticesAsFloat[i].y = 0.0f;
        verticesAsFloat[i + surface->vertexCount].y = 0.0f;
    }

    Vtx* vertices = renderStateRequestVertices(renderState, vertexCount);

    for (int i = 0; i < vertexCount; ++i) {
        Vtx* current = &vertices[i];

        current->v.ob[0] = (short)(verticesAsFloat[i].x * SCENE_SCALE);
        current->v.ob[1] = 1;
        current->v.ob[2] = (short)(verticesAsFloat[i].z * SCENE_SCALE);

        current->v.flag = 0;

        current->v.tc[0] = 0;
        current->v.tc[1] = 0;

        current->v.cn[0] = 255;
        current->v.cn[1] = 255;
        current->v.cn[2] = 255;
        current->v.cn[3] = 255;
    }

    gSPVertex(renderState->dl++, vertices, vertexCount, 0);

    for (int i = 2; i < surface->vertexCount; i += 2) {
        if (i + 1 < surface->vertexCount) {
            gSP2Triangles(renderState->dl++, 0, i - 1, i, 0, 0, i, i + 1, 0);
        } else {
            gSP1Triangle(renderState->dl++, 0, i - 1, i, 0);
        }
    }

    for (int i = 0; i < surface->vertexCount; ++i) { 
        int a = i;
        int b = i + 1;

        if (b == surface->vertexCount) {
            b = 0;
        }

        int c = a + surface->vertexCount;
        int d = b + surface->vertexCount;

        gSP2Triangles(renderState->dl++, a, b, d, 0, a, d, c, 0);
    }
}
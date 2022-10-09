#include "table_surface.h"

#include "../defs.h"

#define MAX_LOOP_SIZE   16

void tableSurfaceRenderLight(struct TableSurfaceMesh* surface, struct Vector3* offset, struct SpotLight* spotLight, struct RenderState* renderState) {
    struct Vector3 pointLoop[MAX_LOOP_SIZE];
    struct Vector3 pointLoopTmp[MAX_LOOP_SIZE];

    for (int i = 0; i < surface->vertexCount; ++i) {
        vector3Add(&surface->vertices[i], offset, &pointLoop[i]);
        vector3Sub(&pointLoop[i], &spotLight->transform.position, &pointLoop[i]);
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

        curr->v.ob[0] = (short)((spotLight->transform.position.x + src[i].x) * SCENE_SCALE);
        curr->v.ob[1] = (short)((spotLight->transform.position.y + src[i].y) * SCENE_SCALE);
        curr->v.ob[2] = (short)((spotLight->transform.position.z + src[i].z) * SCENE_SCALE);

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
#include "shadow_volume_group.h"

#include "../level/level.h"

#include "../build/assets/materials/static.h"
#include "../sk64/skelatool_defs.h"
#include "../math/matrix.h"
#include "../graphics/graphics.h"

Light gNoLight = {{
    {0, 0, 0}, 0,
    {0, 0, 0}, 0,
    {127, 0, 0}, 0
}};

float gLargeSortValue = 10000.0f;

void shadowVolumeGroupInit(struct ShadowVolumeGroup* group, struct Transform* cameraTransform, float (*viewPerspMatrix)[4][4]) {
    group->currentCount = 0;
    group->cameraTransform = cameraTransform;
    group->viewPerspMatrix = viewPerspMatrix;
    group->screenClip.min = gOneVec2;
    group->screenClip.max = gZeroVec2;
}

void shadowVolumeGroupAddSpotLightFace(struct ShadowVolumeGroup* group, struct SpotLight* light, int faceIndex) {
    if (group->currentCount == SHADOW_VOLUME_STEP_MAX_COUNT) {
        return;
    }

    struct ShadowVolumeStep* step = &group->steps[group->currentCount];

    step->sortOrder = spotLightShadowSortOrder(light, faceIndex);
    step->stepType = ShadowVolumeStepTypePlane;
    step->plane.light = light;
    step->plane.faceIndex = faceIndex;

    ++group->currentCount;
}

void shadowVolumeGroupAddObject(struct ShadowVolumeGroup* group, Gfx* displayList, Mtx* armature, Mtx* matrix, Light* light, int materialIndex, float sortOrder) {
    if (group->currentCount == SHADOW_VOLUME_STEP_MAX_COUNT) {
        return;
    }

    struct ShadowVolumeStep* step = &group->steps[group->currentCount];

    step->sortOrder = sortOrder;
    step->stepType = ShadowVolumeStepTypeObject;

    step->object.displayList = displayList;
    step->object.armature = armature;
    step->object.matrix = matrix;
    step->object.light = light ? light : &gNoLight;
    step->object.materialIndex = materialIndex;

    ++group->currentCount;
}

void shadowVolumeGroupSort(struct ShadowVolumeGroup* group, u8* order, u8* tmp, int min, int max) {
    if (max - min <= 1) {
        return;
    }

    int mid = (min + max) >> 1;

    shadowVolumeGroupSort(group, order, tmp, min, mid);
    shadowVolumeGroupSort(group, order, tmp, mid, max);    

    int aIndex = min;
    int bIndex = mid;
    int writeIndex = min;

    while (aIndex < mid || bIndex < max) {
        struct ShadowVolumeStep* aStep = aIndex < mid ? &group->steps[order[aIndex]] : NULL;
        struct ShadowVolumeStep* bStep = bIndex < max ? &group->steps[order[bIndex]] : NULL;

        float aValue = aStep ? aStep->sortOrder : gLargeSortValue;
        float bValue = bStep ? bStep->sortOrder : gLargeSortValue;
        
        if (aValue == bValue) {
            if (aStep->stepType == bStep->stepType) {
                if (aStep->plane.faceIndex < bStep->plane.faceIndex) {
                    --aValue;
                } else {
                    --bValue;
                }
            } else if (aStep->stepType < bStep->stepType) {
                --aValue;
            } else {
                --bValue;
            }
        }

        if (aValue < bValue) {
            tmp[writeIndex] = order[aIndex];
            ++writeIndex;
            ++aIndex;
        } else {
            tmp[writeIndex] = order[bIndex];
            ++writeIndex;
            ++bIndex;
        }
    }

    for (int i = min; i < max; ++i) {
        order[i] = tmp[i];
    }
}

void shadowVolumeGroupRender(struct ShadowVolumeGroup* group, struct RenderState* renderState) {
    u8 order[SHADOW_VOLUME_STEP_MAX_COUNT];
    u8 orderTmp[SHADOW_VOLUME_STEP_MAX_COUNT];

    if (group->currentCount == 0) {
        return;
    }

    for (int i = 0; i < group->currentCount; ++i) {
        order[i] = i;
    }
    
    shadowVolumeGroupSort(group, order, orderTmp, 0, group->currentCount);

    struct ShadowVolumeStep* lastShadowPlane = NULL;

    int currentMaterialIndex = DEFAULT_INDEX;

    gDPSetScissor(
        renderState->dl++, 
        G_SC_NON_INTERLACE,
        MAX(0, (int)group->screenClip.min.x),
        MAX(0, (int)group->screenClip.min.y),
        MIN(SCREEN_WD, (int)group->screenClip.max.x),
        MIN(SCREEN_HT, (int)group->screenClip.max.y)
    );

    gSPDisplayList(renderState->dl++, levelMaterialDefault());

    for (int i = 0; i < group->currentCount; ++i) {
        struct ShadowVolumeStep* currentStep = &group->steps[order[i]];

        int materialIndex;
        Gfx* model = NULL;
        Mtx* transform = NULL;
        Mtx* armature = NULL;
        Light* light = NULL;

        if (currentStep->stepType == ShadowVolumeStepTypePlane) {
            if (lastShadowPlane && 
                lastShadowPlane->plane.light == currentStep->plane.light && 
                lastShadowPlane->plane.faceIndex == currentStep->plane.faceIndex) {
                continue;
            }

            model = spotLightShadowPlane(currentStep->plane.light, currentStep->plane.faceIndex, renderState);

            materialIndex = SHADOW_INDEX;

            lastShadowPlane = currentStep;
        } else {
            materialIndex = currentStep->object.materialIndex;
            transform = currentStep->object.matrix;
            armature = currentStep->object.armature;
            model = currentStep->object.displayList;
            light = currentStep->object.light;
        }

        if (materialIndex != currentMaterialIndex) {
            if (currentMaterialIndex != -1) {
                gSPDisplayList(renderState->dl++, levelMaterialRevert(currentMaterialIndex));
            }
            gSPDisplayList(renderState->dl++, levelMaterial(materialIndex));

            if (materialIndex != SHADOW_INDEX) {
                gDPSetRenderMode(renderState->dl++, G_RM_ZB_OPA_DECAL, G_RM_ZB_OPA_DECAL2);
            }
            
            currentMaterialIndex = materialIndex;
        }

        if (transform) {
            gSPMatrix(renderState->dl++, transform, G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_PUSH);
        }

        if (armature) {
            gSPSegment(renderState->dl++, MATRIX_TRANSFORM_SEGMENT, armature);
        }

        if (light) {
            gSPLight(renderState->dl++, light, 1);
        }

        gSPDisplayList(renderState->dl++, model);

        if (transform) {
            gSPPopMatrix(renderState->dl++, G_MTX_MODELVIEW);
        }
    }

    if (currentMaterialIndex != -1) {
        gSPDisplayList(renderState->dl++, levelMaterialRevert(currentMaterialIndex));
    }

    gDPSetScissor(renderState->dl++, G_SC_NON_INTERLACE, 0, 0, SCREEN_WD, SCREEN_HT);
}

struct SpotLightFace {
    struct SpotLight* spotLight;
    int faceIndex;
};

int spotLightFaceSum(void* data, struct Vector3* direction, struct Vector3* output) {
    struct SpotLightFace* face = (struct SpotLightFace*)data;

    int result = 0;
    float dotCompare = vector3Dot(&face->spotLight->rigidBody.transform.position, direction);

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
            *output = face->spotLight->rigidBody.transform.position;
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

#define FACE_DISTANCE_NONE  -10000.0f

struct CollisionObjectToCamera {
    struct CollisionObject* collisionObject;
    struct Vector3* cameraPosition;
};

int collisionObjectToCameraSum(void* data, struct Vector3* direction, struct Vector3* output) {
    struct CollisionObjectToCamera* collisionObject = (struct CollisionObjectToCamera*)data;

    struct Vector3 offset;

    int result = collisionObject->collisionObject->minkowskiSum(collisionObject->collisionObject->data, direction, output);

    vector3Sub(collisionObject->cameraPosition, output, &offset);

    if (vector3Dot(&offset, direction) > 0.0f) {
        *output = *collisionObject->cameraPosition;
    }

    return result;
}

enum LightIntersection spotLightIsInside(struct ShadowVolumeGroup* group, struct SpotLight* spotLight, struct ShadowVolumeTarget* target, float* furthestBackFace, float* furthestFrontFace) {
    if (!box3DHasOverlap(&spotLight->boundingBox, &target->collisionObject->boundingBox)) {
        return LightIntersectionOutside;
    }

    int isInside = 1;
    float closestBackFace = FACE_DISTANCE_NONE;
    float closestFrontFace = FACE_DISTANCE_NONE;

    struct Simplex simplex;

    struct SpotLightFace spotLightFace;
    spotLightFace.spotLight = spotLight;

    struct Vector3 offset;

    // only add back faces on the first pass
    // if the player found to intersect any faces then
    // a second pass that adds any shadow volumes that
    // occlude the player
    vector3Sub(&spotLight->rigidBody.transform.position, &target->position, &offset);

    for (int i = 0; i < LIGHT_CIRCLE_POINT_COUNT; ++i) {
        spotLightFace.faceIndex = i;

        if (isInside && vector3Dot(&offset, &spotLight->faceNormal[i]) > 0.0f) {
            isInside = 0;
        }

        if (gjkCheckForOverlap(
            &simplex, 
            &spotLightFace, 
            spotLightFaceSum, 
            target->collisionObject, 
            target->collisionObject->minkowskiSum, 
            &spotLight->faceNormal[i]
        )) {
            if (spotLight->isBackFaceMask & (1 << i)) {
                shadowVolumeGroupAddSpotLightFace(group, spotLight, i);

                float faceDepth = spotLightShadowSortOrder(spotLight, i);
                closestBackFace = MAX(closestBackFace, faceDepth);
            }

            isInside = 0;
        }
    }

    if (isInside) {
        return LightIntersectionInside;
    }

    struct CollisionObjectToCamera collisionObjectToCamera;
    collisionObjectToCamera.collisionObject = target->collisionObject;

    // if a back plane is intersecting the player then front
    // planes need to be rendered too
    if (closestBackFace == FACE_DISTANCE_NONE) {
        collisionObjectToCamera.cameraPosition = &target->position;
    } else {
        collisionObjectToCamera.cameraPosition = &group->cameraTransform->position;
    }

    for (int i = 0; i < LIGHT_CIRCLE_POINT_COUNT; ++i) {
        if ((spotLight->isBackFaceMask & (1 << i)) != 0) {
            continue;
        }

        spotLightFace.faceIndex = i;

        if (isInside && vector3Dot(&offset, &spotLight->faceNormal[i]) > 0.0f) {
            isInside = 0;
        }

        if (gjkCheckForOverlap(
            &simplex, 
            &spotLightFace, 
            spotLightFaceSum, 
            &collisionObjectToCamera, 
            collisionObjectToCameraSum, 
            &spotLight->faceNormal[i]
        )) {
            shadowVolumeGroupAddSpotLightFace(group, spotLight, i);

            float faceDepth = spotLightShadowSortOrder(spotLight, i);
            closestFrontFace = MAX(closestFrontFace, faceDepth);

            isInside = 0;
        }
    }

    int result = 0;

    if (closestBackFace != FACE_DISTANCE_NONE) {
        shadowVolumeGroupAddObject(
            group,
            target->displayList,
            target->armature,
            target->matrix,
            target->light,
            target->materialIndex,
            closestBackFace
        );

        result |= LightIntersectionTouchingBackFace;

        *furthestBackFace = MIN(*furthestBackFace, closestBackFace);
    }

    if (closestFrontFace != FACE_DISTANCE_NONE) {
        shadowVolumeGroupAddObject(
            group,
            target->displayList,
            target->armature,
            target->matrix,
            &gNoLight,
            target->materialIndex,
            closestFrontFace
        );

        result |= LightIntersectionTouchingFrontFace;

        *furthestFrontFace= MIN(*furthestFrontFace, closestFrontFace);
    }

    return result;
}

struct Vector3 gLocalOffset = {0.5f, 0.5f, 0.0f};

void shadowVolumeAppendViewPoint(
    struct ShadowVolumeGroup* group,
    struct Vector3* worldPoint
) {
    struct Vector4 pos4D;
    struct Vector3 scaledWorldPoint;
    vector3Scale(worldPoint, &scaledWorldPoint, SCENE_SCALE);
    matrixVec3Mul(*group->viewPerspMatrix, &scaledWorldPoint, &pos4D);
    float invW = 1.0f / pos4D.w;

    struct Vector2 screenPos;
    screenPos.x = (SCREEN_WD * 0.5f) * (pos4D.x * invW + 1.0f);
    screenPos.y = (SCREEN_HT * 0.5f) * (-pos4D.y * invW + 1.0f);

    if (group->screenClip.min.x > group->screenClip.max.x) {
        group->screenClip.min = screenPos;
        group->screenClip.max = screenPos;
    } else {
        vector2Min(&group->screenClip.min, &screenPos, &group->screenClip.min);
        vector2Max(&group->screenClip.max, &screenPos, &group->screenClip.max);
    }
}

Light* shadowVolumeGroupPopulate(
    struct ShadowVolumeGroup* group, 
    struct SpotLight* lights,
    int lightCount,
    struct ShadowVolumeTarget* target
) {
    int startCount = group->currentCount;

    int intersections = 0;

    float furthestBackFace = -FACE_DISTANCE_NONE;
    float furthestFrontFace = -FACE_DISTANCE_NONE;


    for (int i = 0; i < lightCount; ++i) {
        enum LightIntersection spotLightResult = spotLightIsInside(group, &lights[i], target, &furthestBackFace, &furthestFrontFace);

        if (spotLightResult == LightIntersectionInside) {
            group->currentCount = startCount;
            return target->light;
        }

        intersections |= spotLightResult;
    }

    if (intersections == LightIntersectionOutside) {
        return &gNoLight;
    }

    struct Vector3 worldOffset;
    quatMultVector(&group->cameraTransform->rotation, &gLocalOffset, &worldOffset);
    struct Vector3 extent;
    vector3Add(&target->position, &worldOffset, &extent);
    shadowVolumeAppendViewPoint(group, &extent);
    vector3Sub(&target->position, &worldOffset, &extent);
    shadowVolumeAppendViewPoint(group, &extent);

    if (intersections & LightIntersectionTouchingBackFace) {
        return &gNoLight;
    }

    if (furthestFrontFace < furthestBackFace) {
        return target->light;
    }

    return &gNoLight;
}
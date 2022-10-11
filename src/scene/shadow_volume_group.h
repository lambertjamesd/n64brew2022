#ifndef __SCENE_SHADOW_VOLUME_GROUP_H__
#define __SCENE_SHADOW_VOLUME_GROUP_H__

#include "./spot_light.h"

#include "../graphics/renderstate.h"

enum ShadowVolumeStepType {
    ShadowVolumeStepTypePlane,
    ShadowVolumeStepTypeObject,
};

struct ShadowVolumeStep {
    float sortOrder;
    enum ShadowVolumeStepType stepType;

    union {
        struct {
            struct SpotLight* light;
            int faceIndex;
        } plane;

        struct {
            Gfx* displayList;
            Mtx* armature;
            Mtx* matrix;
            Light* light;
            short materialIndex;
        } object;
    };
};

#define SHADOW_VOLUME_STEP_MAX_COUNT    16

struct ShadowVolumeGroup {
    struct ShadowVolumeStep steps[SHADOW_VOLUME_STEP_MAX_COUNT];
    int currentCount;
};

struct ShadowVolumeTarget {
    struct CollisionObject* collisionObject;
    Gfx* displayList;
    Mtx* armature;
    Mtx* matrix;
    Light* light;
    int materialIndex;
    struct Vector3* position;
};

void shadowVolumeGroupInit(struct ShadowVolumeGroup* group);

void shadowVolumeGroupAddSpotLightFace(struct ShadowVolumeGroup* group, struct SpotLight* light, int faceIndex);
void shadowVolumeGroupAddObject(struct ShadowVolumeGroup* group, Gfx* displayList, Mtx* armature, Mtx* matrix, Light* light, int materialIndex, float sortOrder);

void shadowVolumeGroupRender(struct ShadowVolumeGroup* group, struct RenderState* renderState);

Light* shadowVolumeGroupPopulate(
    struct ShadowVolumeGroup* group, 
    struct SpotLight* lights,
    int lightCount,
    struct ShadowVolumeTarget* target
);

#endif
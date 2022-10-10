#ifndef __SCENE_ITEM_H__
#define __SCENE_ITEM_H__

#include "../sk64/skelatool_animator.h"
#include "../sk64/skelatool_armature.h"
#include "../level/level_definition.h"
#include "../graphics/render_scene.h"

#include "../math/transform.h"
#include "../level/level_definition.h"
#include "spot_light.h"

struct ItemTypeDefinition {
    Gfx* dl;
    unsigned short materialIndex;
    unsigned short boneCount;
    unsigned short attachmentCount;
    struct Transform* defaultBones;
    unsigned short* boneParent;
    struct CameraDefinition* cameraDefinition;
    struct Vector3* lightDir;
};

#define ITEM_FLAGS_ATTACHED         (1 << 0)
#define ITEM_FLAGS_HAS_ARMATURE     (1 << 1)
#define ITEM_FLAGS_DROPPED          (1 << 2)
#define ITEM_FLAGS_POOFED           (1 << 3)
#define ITEM_FLAGS_GONE             (1 << 4)
#define ITEM_FLAGS_SUCCESS          (1 << 5)

#define ITEM_PICKUP_RADIUS  0.5f
#define ITEM_DROP_PICKUP_RADIUS  0.75f

extern struct ItemTypeDefinition gItemDefinitions[ItemTypeCount];

struct Item {
    struct Item* next;
    enum ItemType type;
    struct Transform transform;
    struct SKAnimator animator;
    struct SKArmature armature;

    Mtx* mtxTransform;
    Mtx* mtxArmature;

    union
    {
        struct Transform target;
        struct {
            struct Vector3 velocity;
            float pooftimer;
        } dropInfo;
    };
    

    unsigned short flags;
};

void itemInit(struct Item* item, enum ItemType itemType, struct Transform* initialPose);

void itemUpdate(struct Item* item);

void itemPreRender(struct Item* item, struct RenderState* renderState);

void itemRender(struct Item* item, Light* light, struct RenderScene* renderScene);

void itemUpdateTarget(struct Item* item, struct Transform* transform);

void itemMarkNewTarget(struct Item* item);

void itemDrop(struct Item* item);
void itemSuccess(struct Item* item);

struct ItemPool {
    struct Item* itemHead;
    struct Item* unusedHead;
    int itemCount;
};

void itemPoolInit(struct ItemPool* itemPool);

struct Item* itemPoolNew(struct ItemPool* itemPool, enum ItemType itemType, struct Transform* initialPose);
void itemPoolFree(struct ItemPool* itemPool, struct Item* item);

void itemPoolUpdate(struct ItemPool* itemPool);

void itemPoolRender(struct ItemPool* itemPool, struct SpotLight* spotLights, int spotLightCount, struct RenderScene* renderScene);

#endif
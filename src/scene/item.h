#ifndef __SCENE_ITEM_H__
#define __SCENE_ITEM_H__

#include "../sk64/skelatool_animator.h"
#include "../sk64/skelatool_armature.h"
#include "../level/level_definition.h"
#include "../graphics/render_scene.h"

#include "../math/transform.h"
#include "../level/level_definition.h"
#include "spot_light.h"

enum ItemType {
    ItemTypePumpkin,

    ItemTypeCount,
};

struct ItemTypeDefinition {
    Gfx* dl;
    unsigned short materialIndex;
    unsigned short boneCount;
    unsigned short attachmentCount;
    struct Transform* defaultBones;
    unsigned short* boneParent;
};

#define ITEM_FLAGS_ATTACHED (1 << 0)
#define ITEM_FLAGS_HAS_ARMATURE (1 << 1)

struct Item {
    struct Item* next;
    enum ItemType type;
    struct Transform transform;
    struct SKAnimator animator;
    struct SKArmature armature;

    struct Transform target;

    unsigned short flags;
};

void itemInit(struct Item* item, enum ItemType itemType, struct Transform* initialPose);

void itemUpdate(struct Item* item);

void itemRender(struct Item* item, struct RenderScene* renderScene);

void itemUpdateTarget(struct Item* item, struct Transform* transform);

void itemMarkNewTarget(struct Item* item);

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
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
    ItemTypeHat,
    ItemTypeBrain,
    ItemTypeBroom,
    ItemTypeCandle,
    ItemTypeCat,
    ItemTypeCobweb,
    ItemTypeCrow,
    ItemTypeHand,
    ItemTypeRat,
    ItemTypeScarecrow,
    ItemTypeSkull,
    ItemTypeSpider,

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

#define ITEM_FLAGS_ATTACHED         (1 << 0)
#define ITEM_FLAGS_HAS_ARMATURE     (1 << 1)
#define ITEM_FLAGS_DROPPED          (1 << 2)
#define ITEM_FLAGS_POOFED           (1 << 3)
#define ITEM_FLAGS_GONE             (1 << 4)

#define ITEM_PICKUP_RADIUS  0.5f

struct Item {
    struct Item* next;
    enum ItemType type;
    struct Transform transform;
    struct SKAnimator animator;
    struct SKArmature armature;

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

void itemRender(struct Item* item, Light* light, struct RenderScene* renderScene);

void itemUpdateTarget(struct Item* item, struct Transform* transform);

void itemMarkNewTarget(struct Item* item);

void itemDrop(struct Item* item);

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
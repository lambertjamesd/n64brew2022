#ifndef __SCENE_ITEM_REQUESTER_H__
#define __SCENE_ITEM_REQUESTER_H__

#include "../math/transform.h"
#include "item.h"
#include "../level/level_definition.h"
#include "../graphics/render_scene.h"
#include "../collision/collision_object.h"
#include "../sk64/skelatool_animator.h"
#include "../sk64/skelatool_armature.h"

enum ItemRequesterFlags {
    ItemRequesterFlagsHover = (1 << 0),
};

struct ItemRequester {
    struct Transform transform;
    struct CollisionCapsule collisionCapsule;
    struct SKAnimator animator;
    struct SKArmature armature;
    enum ItemType requestedType;
    float timeLeft;
    float duration;
    float requestDelay;
    short flags;
    ALSndId endingSoundId;
};

enum ItemDropResult {
    ItemDropResultNone,
    ItemDropResultSuccess,
    ItemDropResultFail,
};

void itemRequesterInit(struct ItemRequester* requester, struct ItemRequesterDefinition* definition);
int itemRequesterUpdate(struct ItemRequester* requester, float timeScale);
void itemRequesterRenderGenerate(struct ItemRequester* requester, int itemIndex, struct RenderState* renderState);
void itemRequesterRender(struct ItemRequester* requester, int itemIndex, struct RenderScene* scene);

void itemRequesterRequestItem(struct ItemRequester* requester, enum ItemType itemType, float duration);
int itemRequesterIsActive(struct ItemRequester* requester);

int itemRequesterHover(struct ItemRequester* requester, struct Item* item, struct Vector3* dropAt);

enum ItemDropResult itemRequesterDrop(struct ItemRequester* requester, struct Item* item, struct Vector3* dropAt);

#endif
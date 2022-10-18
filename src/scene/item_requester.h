#ifndef __SCENE_ITEM_REQUESTER_H__
#define __SCENE_ITEM_REQUESTER_H__

#include "../math/transform.h"
#include "item.h"
#include "../level/level_definition.h"
#include "../graphics/render_scene.h"
#include "../collision/collision_object.h"

enum ItemRequesterFlags {
    ItemRequesterFlagsHover = (1 << 0),
};

struct ItemRequester {
    struct Transform transform;
    struct CollisionCapsule collisionCapsule;
    enum ItemType requestedType;
    float timeLeft;
    float duration;
    float requestDelay;
    short flags;
};

enum ItemDropResult {
    ItemDropResultNone,
    ItemDropResultSuccess,
    ItemDropResultFail,
};

void itemRequesterInit(struct ItemRequester* requester, struct ItemRequesterDefinition* definition);
void itemRequesterUpdate(struct ItemRequester* requester);
void itemRequesterRender(struct ItemRequester* requester, struct RenderScene* scene);

void itemRequesterRequestItem(struct ItemRequester* requester, enum ItemType itemType, float duration);
int itemRequesterIsActive(struct ItemRequester* requester);

int itemRequesterHover(struct ItemRequester* requester, struct Item* item, struct Vector3* dropAt);

enum ItemDropResult itemRequesterDrop(struct ItemRequester* requester, struct Item* item, struct Vector3* dropAt);

#endif
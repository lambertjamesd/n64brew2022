#ifndef __SCENE_ITEM_REQUESTER_H__
#define __SCENE_ITEM_REQUESTER_H__

#include "../math/transform.h"
#include "item.h"
#include "../level/level_definition.h"
#include "../graphics/render_scene.h"

struct ItemRequester {
    struct Transform transform;
    enum ItemType requestedType;
    float timeLeft;
};

void itemRequesterInit(struct ItemRequester* requester, struct ItemRequesterDefinition* definition);
void itemRequesterUpdate(struct ItemRequester* requester);
void itemRequesterRender(struct ItemRequester* requester, struct RenderScene* scene);

#endif
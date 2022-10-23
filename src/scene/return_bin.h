#ifndef __SCENE_RETURN_BIN_H__
#define __SCENE_RETURN_BIN_H__

#include "../level/level_definition.h"
#include "../graphics/renderstate.h"
#include "../graphics/render_scene.h"
#include "../collision/collision_object.h"
#include "item.h"

struct ReturnBin {
    struct Vector3 position;
    struct CollisionCapsule collisionCapsule;
};

void returnBinInit(struct ReturnBin* returnBin, struct ReturnBinDefinition* definition);
int returnBinDropItem(struct ReturnBin* returnBin, struct Item* item, struct Vector3* dropAt);
int returnBinHover(struct ReturnBin* returnBin, struct Vector3* dropAt, struct Vector3* hoverOutput);

void returnBinRender(struct ReturnBin* returnBin, struct RenderScene* renderScene);

#endif
#ifndef __SCENE_CONVEYOR_H__
#define __SCENE_CONVEYOR_H__

#include "../math/transform.h"
#include "../graphics/render_scene.h"

#include "../level/level_definition.h"

#include "item.h"

struct Conveyor {
    struct Transform transform;
    struct Item* pendingItems[2];
    float beltOffset[2];
};

void conveyorInit(struct Conveyor* conveyor, struct ConveyorDefinition* definition);
void conveyorUpdate(struct Conveyor* conveyor);
void conveyorRender(struct Conveyor* conveyor, struct RenderScene* renderScene);

int conveyorCanAcceptItem(struct Conveyor* conveyor);
void conveyorAcceptItem(struct Conveyor* conveyor, struct Item* item);

struct Item* conveyorReleaseItem(struct Conveyor* conveyor);

struct Item* conveyorPickupItem(struct Conveyor* conveyor, struct Vector3* grabPosition);

#endif
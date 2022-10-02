#ifndef __SCENE_TABLE_H__
#define __SCENE_TABLE_H__

#include <ultra64.h>

#include "../graphics/render_scene.h"
#include "../math/vector3.h"
#include "item.h"
#include "../level/level_definition.h"
#include "table_type.h"
#include "../collision/collision_object.h"

struct Table {
    struct Vector3 position;
    struct TableType* tableType;
    struct Item** itemSlots;
    struct CollisionObject collisionObject;
};

void tableInit(struct Table* table, struct TableDefinition* def);

void tableRender(struct Table* table, struct RenderScene* renderScene);

struct Item* tablePickupItem(struct Table* table, struct Vector3* grabFrom);

int tableDropItem(struct Table* table, struct Item* item, struct Vector3* dropAt);

#endif
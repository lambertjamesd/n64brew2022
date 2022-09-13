#ifndef __ITEM_SLOT_H__
#define __ITEM_SLOT_H__

#include "../math/vector3.h"
#include "../level/level_definition.h"

struct ItemSlot {
    struct Vector3 position;
};

void itemSlotInit(struct ItemSlot* itemSlot, struct ItemSlotDefinition* definition);

#endif
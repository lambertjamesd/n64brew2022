#include "item_slot.h"

void itemSlotInit(struct ItemSlot* itemSlot, struct ItemSlotDefinition* definition) {
    itemSlot->position = definition->position;
}
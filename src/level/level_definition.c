#include "level_definition.h"


#define ADJUST_POINTER_POS(ptr, offset) (void*)((ptr) ? (char*)(ptr) + (offset) : 0)

struct LevelDefinition* levelFixPointers(struct LevelDefinition* from, int pointerOffset) {
    struct LevelDefinition* result = ADJUST_POINTER_POS(from, pointerOffset);

    result->staticContent = ADJUST_POINTER_POS(result->staticContent, pointerOffset);
    result->groundContent = ADJUST_POINTER_POS(result->groundContent, pointerOffset);
    result->itemSlots = ADJUST_POINTER_POS(result->itemSlots, pointerOffset);
    result->spotLights = ADJUST_POINTER_POS(result->spotLights, pointerOffset);

    return result;
}
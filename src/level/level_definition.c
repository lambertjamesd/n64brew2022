#include "level_definition.h"

#include "../util/rom.h"

struct LevelDefinition* levelFixPointers(struct LevelDefinition* from, int pointerOffset) {
    struct LevelDefinition* result = ADJUST_POINTER_POS(from, pointerOffset);

    result->staticContent = ADJUST_POINTER_POS(result->staticContent, pointerOffset);
    result->groundContent = ADJUST_POINTER_POS(result->groundContent, pointerOffset);
    result->itemSlots = ADJUST_POINTER_POS(result->itemSlots, pointerOffset);
    result->spotLights = ADJUST_POINTER_POS(result->spotLights, pointerOffset);
    result->conveyors = ADJUST_POINTER_POS(result->conveyors, pointerOffset);
    result->tables = ADJUST_POINTER_POS(result->tables, pointerOffset);
    result->itemRequesters = ADJUST_POINTER_POS(result->itemRequesters, pointerOffset);

    result->script = ADJUST_POINTER_POS(result->script, pointerOffset);
    result->script->steps = ADJUST_POINTER_POS(result->script->steps, pointerOffset);

    result->boundary = ADJUST_POINTER_POS(result->boundary, pointerOffset);

    for (int i = 0; i < result->script->stepCount; ++i) {
        result->script->steps[i].itemPool = ADJUST_POINTER_POS(result->script->steps[i].itemPool, pointerOffset);
    }

    return result;
}
#include "level.h"

#include "../build/assets/materials/static.h"
#include "../build/assets/levels/level_list.h"

#include "../util/memory.h"
#include "../util/rom.h"

#include "../graphics/graphics.h"

struct LevelDefinition* gCurrentLevel;
int gCurrentLevelIndex;
int gQueuedLevel = NO_QUEUED_LEVEL;

int levelMaterialTransparentStart() {
    return STATIC_TRANSPARENT_START;
}

Gfx* levelMaterialDefault() {
    return levelMaterial(DEFAULT_INDEX);
}

Gfx* levelMaterial(int index) {
    return ((Gfx**)ADJUST_POINTER_FOR_SEGMENT(static_material_list, gMaterialSegment, MATERIAL_SEGMENT))[index];
}

Gfx* levelMaterialRevert(int index) {
    return ((Gfx**)ADJUST_POINTER_FOR_SEGMENT(static_material_revert_list, gMaterialSegment, MATERIAL_SEGMENT))[index];
}

void loadLevel(unsigned index) {
    if (index >= LEVEL_COUNT) {
        return;
    }

    struct LevelMetadata* metadata = &gLevelList[index];

    void* memory = malloc(metadata->segmentRomEnd - metadata->segmentRomStart);
    romCopy(metadata->segmentRomStart, memory, metadata->segmentRomEnd - metadata->segmentRomStart);

    gLevelSegment = memory;

    gCurrentLevel = levelFixPointers(metadata->levelDefinition, (char*)memory - metadata->segmentStart);
    gCurrentLevelIndex = index;
    gQueuedLevel = NO_QUEUED_LEVEL;
}

void levelQueueLoad(int index) {
    if (index == NEXT_LEVEL) {
        gQueuedLevel = gCurrentLevelIndex + 1;
    } else {
        gQueuedLevel = index;
    }
}

int levelGetQueued() {
    return gQueuedLevel;
}
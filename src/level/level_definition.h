#ifndef __LEVEL_DEFINITION_H__
#define __LEVEL_DEFINITION_H__

#include <ultra64.h>

struct StaticContentElement {
    Gfx* displayList;
    u8 materialIndex;
};

struct LevelDefinition {
    struct StaticContentElement* staticContent;
    short staticContentCount;
};

struct LevelDefinition* levelFixPointers(struct LevelDefinition* from, int pointerOffset);

#endif
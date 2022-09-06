#ifndef __LEVEL_LEVEL_H__
#define __LEVEL_LEVEL_H__

#include <ultra64.h>

#include "level_definition.h"

#define NO_QUEUED_LEVEL -2
#define NEXT_LEVEL      -1

extern struct LevelDefinition* gCurrentLevel;
extern int gCurrentLevelIndex;
extern int gQueuedLevel;

int levelMaterialTransparentStart();
int levelMaterialDefault();

Gfx* levelMaterial(int index);
Gfx* levelMaterialRevert(int index);

#endif
#ifndef __LEVEL_LEVEL_H__
#define __LEVEL_LEVEL_H__

#include <ultra64.h>

#include "level_definition.h"
#include "level_metadata.h"

#define MAIN_MENU_LEVEL -3
#define NO_QUEUED_LEVEL -2
#define NEXT_LEVEL      -1

extern struct LevelDefinition* gCurrentLevel;
extern int gCurrentLevelIndex;
extern int gQueuedLevel;

int levelMaterialTransparentStart();
Gfx* levelMaterialDefault();

void loadLevel(unsigned index);
void levelQueueLoad(int index);
int levelGetQueued();

Gfx* levelMaterial(int index);
Gfx* levelMaterialRevert(int index);

struct LevelMetadata* levelGetMetadata(int index);
int levelGetCount();

#endif
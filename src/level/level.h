#ifndef __LEVEL_LEVEL_H__
#define __LEVEL_LEVEL_H__

#include <ultra64.h>

int levelMaterialTransparentStart();
int levelMaterialDefault();

Gfx* levelMaterial(int index);
Gfx* levelMaterialRevert(int index);

#endif
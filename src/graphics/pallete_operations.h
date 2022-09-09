#ifndef __PALLETE_OPERATIONS_H__
#define __PALLETE_OPERATIONS_H__

#include "./renderstate.h"
#include "./color.h"

u16* palleteGenerateLit(struct Coloru8* colors, struct Coloru8 ambientLight, struct Coloru8 lightColor, struct RenderState* renderState);

#endif
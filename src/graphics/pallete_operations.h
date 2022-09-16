#ifndef __PALLETE_OPERATIONS_H__
#define __PALLETE_OPERATIONS_H__

#include "./renderstate.h"
#include "./color.h"

u16* palleteGenerateLit(struct Coloru8* colors, struct Colorf32* ambientLight, struct Colorf32* ambientScale, struct Colorf32* lightColor, struct RenderState* renderState);

#endif
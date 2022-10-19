#ifndef __PALLETE_OPERATIONS_H__
#define __PALLETE_OPERATIONS_H__

#include "./renderstate.h"
#include "./color.h"

enum PalleteEffects {
    PalleteEffectsGrayscaleRed = (1 << 0),
};

u16* palleteGenerateLit(struct Coloru8* colors, struct Colorf32* ambientLight, struct Colorf32* ambientScale, struct Colorf32* lightColor, enum PalleteEffects effects, float fadeAmount, struct RenderState* renderState);

#endif
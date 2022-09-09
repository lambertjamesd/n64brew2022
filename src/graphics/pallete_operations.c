#include "pallete_operations.h"

u16* palleteGenerateLit(struct Coloru8* colors, struct Coloru8 ambientLight, struct Coloru8 lightColor, struct RenderState* renderState) {
    u16* result = renderStateRequestMemory(renderState, sizeof(u16) * 256);

    u16* output = result;

    for (unsigned i = 0; i < 256; ++i) {
        struct Coloru8 colorOut;
        colorU8Mul(colors, &ambientLight, &colorOut);

        *output = GPACK_RGBA5551(colorOut.r, colorOut.g, colorOut.b, 1);
        ++output;

        colorU8Mul(colors, &lightColor, &colorOut);

        *output = GPACK_RGBA5551(colorOut.r, colorOut.g, colorOut.b, 1);
        ++output;

        ++colors;
    }

    return result;
}
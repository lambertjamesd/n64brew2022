#include "pallete_operations.h"

u8 palleteProcessChannel(u8 input, float add, float scale) {
    float result = (((float)input * (1.0f / 255.0f)) + add) * scale;

    if (result > 1.0f) {
        return 255;
    } else if (result < 0.0f) {
        return 0;
    } else {
        return (u8)(result * 255.0f);
    }
}

struct Coloru8 palleteProcessColor(struct Coloru8 input, struct Colorf32* preAdd, struct Colorf32* postScale) {
    struct Coloru8 result;
    result.r = palleteProcessChannel(input.r, preAdd->r, postScale->r);
    result.g = palleteProcessChannel(input.g, preAdd->g, postScale->g);
    result.b = palleteProcessChannel(input.b, preAdd->b, postScale->b);
    result.a = input.a;
    return result;
} 

u16* palleteGenerateLit(struct Coloru8* colors, struct Colorf32* ambientLight, struct Colorf32* ambientScale, struct Colorf32* lightColor, struct RenderState* renderState) {
    u16* result = renderStateRequestMemory(renderState, sizeof(u16) * 256);

    u16* output = result;

    struct Colorf32 lightScale;

    float max = MAX(lightColor->r, lightColor->g);
    max = MAX(lightColor->g, lightColor->b);

    float scale = 1.0f / (1.0f + max);

    lightScale.r = scale;
    lightScale.g = scale;
    lightScale.b = scale;
    lightScale.a = 1.0f;

    for (unsigned i = 0; i < 80; ++i) {
        struct Coloru8 colorOut = palleteProcessColor(*colors, ambientLight, ambientScale);

        *output = GPACK_RGBA5551(colorOut.r, colorOut.g, colorOut.b, 1);
        ++output;

        colorOut = palleteProcessColor(*colors, lightColor, &lightScale);

        *output = GPACK_RGBA5551(colorOut.r, colorOut.g, colorOut.b, 1);
        ++output;

        *output = GPACK_RGBA5551(colorOut.r, colorOut.g, colorOut.b, 1);
        ++output;

        *output = GPACK_RGBA5551(colorOut.r, colorOut.g, colorOut.b, 1);
        ++output;

        ++colors;
    }

    return result;
}
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

struct Coloru8 palleteApplyEffects(struct Coloru8 color, enum PalleteEffects effects, int srcIndex) {
    if (effects & PalleteEffectsGrayscaleRed) {
        if (srcIndex % 12 != 4) {
            int grayscale = (54 * color.r + 182 * color.g + 18) >> 8;

            color.r = grayscale;
            color.g = grayscale;
            color.b = grayscale;
        }
    }

    return color;
}

u16* palleteGenerateLit(struct Coloru8* colors, struct Colorf32* ambientLight, struct Colorf32* ambientScale, struct Colorf32* lightColor, enum PalleteEffects effects, float fadeAmount, struct RenderState* renderState) {
    u16* result = renderStateRequestMemory(renderState, sizeof(u16) * 256);

    u16* output = result;

    struct Colorf32 lightScale;

    float max = MAX(lightColor->r, lightColor->g);
    max = MAX(lightColor->g, lightColor->b);

    float scale = fadeAmount / (1.0f + max);

    struct Colorf32 finalScale = *ambientScale;

    finalScale.r *= fadeAmount;
    finalScale.g *= fadeAmount;
    finalScale.b *= fadeAmount;

    lightScale.r = scale;
    lightScale.g = scale;
    lightScale.b = scale;
    lightScale.a = 1.0f;

    for (unsigned i = 0; i < 80; ++i) {
        struct Coloru8 colorOut = palleteApplyEffects(palleteProcessColor(*colors, ambientLight, &finalScale), effects, i);

        *output = GPACK_RGBA5551(colorOut.r, colorOut.g, colorOut.b, 1);
        ++output;

        colorOut = palleteApplyEffects(palleteProcessColor(*colors, lightColor, &lightScale), effects, i);

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
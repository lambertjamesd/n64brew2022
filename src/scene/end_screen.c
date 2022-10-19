#include "end_screen.h"

#include "../build/assets/materials/ui.h"
#include "../graphics/graphics.h"
#include "../ui/spritefont.h"
#include "../ui/nightchilde.h"
#include "../util/time.h"
#include "../math/mathf.h"

void endScreenInit(struct EndScreen* endScreen) { 
    endScreen->animationLerp = 0.0f;
    endScreen->success = -1;
    endScreen->preDelay = 1.0f;
    endScreen->textAnimation = 0.0f;
}

struct Coloru8 gEndScreenBlack = {0, 0, 0, 255};
struct Coloru8 gEndScreenRed = {200, 0, 0, 255};

#define END_SCREEN_WIDTH    512
#define END_SCREEN_HEIGHT   144

#define END_SCREEN_RED_OFFSET   24

#define FIRST_FALL_TIME     1.5f
#define FIRST_FALL_HEIGHT   600.0f

#define TIME_PER_CHARACTER  0.5f

#define SECOND_FALL_TIME    1.4f
#define SECOND_FALL_HEIGHT  200.0f

#define THIRD_FALL_TIME    0.6f
#define THIRD_FALL_HEIGHT  60.0f

void endScreenTextModifier(void* data, int index, char character, int* x, int* y, struct Coloru8* color) {
    struct EndScreen* endScreen = (struct EndScreen*)data;

    float animationTime = endScreen->textAnimation - index * TIME_PER_CHARACTER;

    if (animationTime < 0.0f) {
        animationTime = 0.0f;
    }

    *x += randomInRange(-1, 2);
    *y += randomInRange(-1, 2);

    if (animationTime < FIRST_FALL_TIME) {
        *y += animationTime * animationTime * (FIRST_FALL_HEIGHT * (1.0f / FIRST_FALL_TIME) * (1.0f / FIRST_FALL_TIME)) - FIRST_FALL_HEIGHT;
        return;
    }

    animationTime -= FIRST_FALL_TIME;

    if (animationTime < SECOND_FALL_TIME) {
        float timeLerp = (animationTime - 0.5f * SECOND_FALL_TIME) * (2.0f / SECOND_FALL_TIME);
        *y += timeLerp * timeLerp * SECOND_FALL_HEIGHT - SECOND_FALL_HEIGHT;
        return;
    }

    animationTime -= SECOND_FALL_TIME;

    if (animationTime < THIRD_FALL_TIME) {
        float timeLerp = (animationTime - 0.5f * THIRD_FALL_TIME) * (2.0f / THIRD_FALL_TIME);
        *y += timeLerp * timeLerp * THIRD_FALL_HEIGHT - THIRD_FALL_HEIGHT;
    }
}

void endScreenRender(struct EndScreen* endScreen, struct RenderState* renderState) {
    // if (endScreen->success == -1) {
    //     return;
    // }

    if (endScreen->preDelay > 0.0f) {
        return;
    }

    int screenX = (SCREEN_WD - END_SCREEN_WIDTH) / 2;
    int screenY = (SCREEN_HT - END_SCREEN_HEIGHT) / 2;

    spriteSetColor(
        renderState, 
        SOLID_UI_INDEX, 
        gEndScreenRed
    );

    spriteSolid(
        renderState,
        SOLID_UI_INDEX,
        screenX - END_SCREEN_RED_OFFSET,
        screenY + END_SCREEN_HEIGHT,
        END_SCREEN_WIDTH,
        END_SCREEN_RED_OFFSET
    );

    spriteSolid(
        renderState,
        SOLID_UI_INDEX,
        screenX - END_SCREEN_RED_OFFSET,
        screenY + END_SCREEN_RED_OFFSET,
        END_SCREEN_RED_OFFSET,
        END_SCREEN_HEIGHT - END_SCREEN_RED_OFFSET
    );

    spriteSetColor(
        renderState, 
        SOLID_UI_INDEX, 
        gEndScreenBlack
    );

    spriteSolid(
        renderState,
        SOLID_UI_INDEX,
        screenX,
        screenY,
        END_SCREEN_WIDTH,
        END_SCREEN_HEIGHT
    );

    fontRenderText(
        renderState,
        &gNightChilde,
        "terminated",
        screenX + 64,
        screenY + 42,
        2,
        endScreen,
        endScreenTextModifier
    );
}

void endScreenUpdate(struct EndScreen* endScreen) {
    // if (endScreen->success == -1) {
    //     return;
    // }

    if (endScreen->preDelay > 0) {
        endScreen->preDelay -= FIXED_DELTA_TIME;
        return;
    }

    if (endScreen->animationLerp < 1.0) {
        endScreen->animationLerp += FIXED_DELTA_TIME;

        if (endScreen->animationLerp > 1.0f) {
            endScreen->animationLerp = 1.0f;
        }

        return;
    }

    endScreen->textAnimation += FIXED_DELTA_TIME;
}

void endScreenEndame(struct EndScreen* endScreen, int success) {
    endScreen->success = success;
}
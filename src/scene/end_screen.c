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

#define FADE_IN_TIME    1.0f

#define FIRST_FALL_TIME     1.5f
#define FIRST_FALL_HEIGHT   600.0f

#define TIME_PER_CHARACTER  0.3f

#define SECOND_FALL_TIME    1.4f
#define SECOND_FALL_HEIGHT  200.0f

#define THIRD_FALL_TIME    0.6f
#define THIRD_FALL_HEIGHT  60.0f

#define EXPLODE_TEXT_DELAY 9.0f

#define EXPLODE_HEIGHT      100.0f
#define EXPLODE_DURATION    1.4f

#define TOTAL_ANIMATION_TIME    13.0f

void endScreenTextModifier(void* data, int index, char character, int* x, int* y, struct Coloru8* color) {
    struct EndScreen* endScreen = (struct EndScreen*)data;

    float animationTime = endScreen->textAnimation - ((index * 13) % 10) * TIME_PER_CHARACTER;

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

    float explodeTime = (endScreen->textAnimation - EXPLODE_TEXT_DELAY) - fabsf(index - 4.5f) * TIME_PER_CHARACTER;

    if (explodeTime < 0.0f) {
        return;
    }

    float timeLerp = (explodeTime - 0.5f * EXPLODE_DURATION) * (2.0f / EXPLODE_DURATION);

    int offset = timeLerp * timeLerp * EXPLODE_HEIGHT - EXPLODE_HEIGHT;

    if (offset > 300) {
        offset = 300;
    }

    *y += offset;
}

void endScreenRender(struct EndScreen* endScreen, struct RenderState* renderState) {
    // if (endScreen->success == -1) {
    //     return;
    // }

    if (endScreen->preDelay > 0.0f) {
        return;
    }

    int screenX = (SCREEN_WD - END_SCREEN_WIDTH) / 2;
    int screenY = (SCREEN_HT - END_SCREEN_HEIGHT) / 2 - SCREEN_HT * (1.0f - endScreen->animationLerp);

    int redScreenY = (SCREEN_HT - END_SCREEN_HEIGHT) / 2 + SCREEN_WD * (1.0f - endScreen->animationLerp);

    spriteSetColor(
        renderState, 
        SOLID_UI_INDEX, 
        gEndScreenRed
    );

    int yOverlap = (screenY + END_SCREEN_HEIGHT) - redScreenY;

    if (yOverlap < 0) {
        yOverlap = 0;
    }

    spriteSolid(
        renderState,
        SOLID_UI_INDEX,
        screenX - END_SCREEN_RED_OFFSET,
        redScreenY + yOverlap,
        END_SCREEN_WIDTH,
        END_SCREEN_RED_OFFSET + END_SCREEN_HEIGHT - yOverlap
    );

    spriteSolid(
        renderState,
        SOLID_UI_INDEX,
        screenX - END_SCREEN_RED_OFFSET,
        redScreenY + END_SCREEN_RED_OFFSET,
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

    if (endScreen->textAnimation < TOTAL_ANIMATION_TIME) {
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
}

void endScreenUpdate(struct EndScreen* endScreen) {
    // if (endScreen->success == -1) {
    //     return;
    // }

    if (endScreen->preDelay > 0) {
        endScreen->preDelay -= FIXED_DELTA_TIME;
        return;
    }

    if (endScreen->textAnimation < TOTAL_ANIMATION_TIME && endScreen->animationLerp < 1.0f) {
        endScreen->animationLerp += FIXED_DELTA_TIME * (1.0f / FADE_IN_TIME);

        if (endScreen->animationLerp > 1.0f) {
            endScreen->animationLerp = 1.0f;
        }
    }

    if (endScreen->textAnimation > TOTAL_ANIMATION_TIME && endScreen->animationLerp > 0.0f) {
        endScreen->animationLerp -= FIXED_DELTA_TIME * (1.0f / FADE_IN_TIME);

        if (endScreen->animationLerp < 0.0f) {
            endScreen->animationLerp = 0.0f;
        }
    }

    endScreen->textAnimation += FIXED_DELTA_TIME;
}

void endScreenEndame(struct EndScreen* endScreen, int success) {
    endScreen->success = success;
}

int endSceenIsDone(struct EndScreen* endScreen) {
    return endScreen->textAnimation >= TOTAL_ANIMATION_TIME && endScreen->animationLerp <= 0.0f;
}


float endScreenFadeAmount(struct EndScreen* endScreen) {
    if (endScreen->textAnimation < TOTAL_ANIMATION_TIME) {
        return 1.0f - 0.5f * endScreen->animationLerp;
    }

    return 0.5f * endScreen->animationLerp;
}
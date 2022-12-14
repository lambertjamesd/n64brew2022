#include "tutorial.h"

#include "../ui/nightchilde.h"

#include "../build/assets/materials/ui.h"
#include "../graphics/graphics.h"
#include "../util/time.h"
#include "../math/vector2.h"
#include "../math/mathf.h"
#include "../controls/controller.h"

#include "../build/src/audio/clips.h"
#include "../audio/soundplayer.h"

#include <string.h>

#define TRANSITION_TIME 0.5f

#define CHARACTERS_PER_SECOND   20.0f
#define SLOW_CHARACTERS_PER_SECOND   10.0f

#define CHARACTER_ANIMATION_OFFSET    4.0f

#define SHAKE_PERIOD    0.1f

struct TutorialDialogStep* tutorialCurrentDialogStep(struct Tutorial* tutorial) {
    if (!tutorial->currentStep) {
        return NULL;
    }

    if (tutorial->currentDialogIndex >= tutorial->currentStep->dialogCount) {
        return NULL;
    }

    return &tutorial->currentStep->dialog[tutorial->currentDialogIndex];
}

void tutorialInitCurrentMessage(struct Tutorial* tutorial) {
    tutorial->currentDialogCharacterCount = 0;
    tutorial->currentCharacter = 0.0f;
    tutorial->currentTextDelay = 0.0f;
    tutorial->textEffects = 0;

    struct TutorialDialogStep* dialog = tutorialCurrentDialogStep(tutorial);

    if (!dialog) {
        return;
    }

    tutorial->currentTextDelay = dialog->preDelay;
    tutorial->textEffects = dialog->effects;
    tutorial->currentDialogCharacterCount = strlen(dialog->message);
}

void tutorialSetNextStep(struct Tutorial* tutorial, short index) {
    if (index == TUTORAL_NO_STEP) {
        tutorial->currentStep = NULL;
    } else {
        tutorial->currentStep = &tutorial->tutorial[index];
        tutorial->currentDialogIndex = 0;
        tutorial->currentPrompt = tutorial->currentStep->prompt;
    }

    tutorialInitCurrentMessage(tutorial);
}

void tutorialInit(struct Tutorial* tutorial, struct TutorialStep* script, short tutorialOnStart) {
    tutorial->tutorial = script;
    tutorial->currentStep = NULL;
    tutorial->currentDialogIndex = 0;
    tutorial->currentDialogCharacterCount = 0;
    tutorial->currentCharacter = 0;
    tutorial->currentTextDelay = 0;
    tutorial->animationLerp = 0.0f;
    tutorial->promptBoxLerp = 0.0f;

    tutorial->talkingSound = SOUND_ID_NONE;

    tutorialSetNextStep(tutorial, tutorialOnStart);
}

void tutorialItemPickedUp(struct Tutorial* tutorial) {
    if (tutorial->currentStep && tutorial->currentStep->onPickup != TUTORAL_NO_STEP) {
        tutorialSetNextStep(tutorial, tutorial->currentStep->onPickup);
    }
}

void tutorialItemDropped(struct Tutorial* tutorial, enum TutorialDropType dropType) {
    if (!tutorial->currentStep) {
        return;
    }

    switch (dropType) {
        case TutorialDropTypeSuccess:
            if (tutorial->currentStep->onSuccess != TUTORAL_NO_STEP) {
                tutorialSetNextStep(tutorial, tutorial->currentStep->onSuccess);
            }
            return;
        case TutorialDropTypeSuccessThrow:
            if (tutorial->currentStep->onSuccessThrow != TUTORAL_NO_STEP) {
                tutorialSetNextStep(tutorial, tutorial->currentStep->onSuccessThrow);
            } else if (tutorial->currentStep->onSuccess != TUTORAL_NO_STEP) {
                tutorialSetNextStep(tutorial, tutorial->currentStep->onSuccess);
            }
            return;
        case TutorialDropTypeFail:
            if (tutorial->currentStep->onFail != TUTORAL_NO_STEP) {
                tutorialSetNextStep(tutorial, tutorial->currentStep->onFail);
            }
            return;
        case TutorialDropTypeTable:
            if (tutorial->currentStep->onTable != TUTORAL_NO_STEP) {
                tutorialSetNextStep(tutorial, tutorial->currentStep->onTable);
            }
            return;
    }
}

struct TutorialDialogStep* tutorialCurrentDialog(struct Tutorial* tutorial) {
    if (!tutorial->currentStep) {
        return NULL;
    }

    if (tutorial->currentDialogIndex >= tutorial->currentStep->dialogCount) {
        return NULL;
    }

    return &tutorial->currentStep->dialog[tutorial->currentDialogIndex];
}

int tutorialUpdate(struct Tutorial* tutorial) {
    tutorial->animationLerp = mathfMoveTowards(
        tutorial->animationLerp,
        tutorial->currentStep && tutorial->currentDialogIndex < tutorial->currentStep->dialogCount ? 1.0f : 0.0f,
        FIXED_DELTA_TIME / TRANSITION_TIME
    );


    tutorial->promptBoxLerp = mathfMoveTowards(
        tutorial->promptBoxLerp,
        tutorial->currentStep && tutorial->currentStep->prompt != TutorialPromptTypeNone ? 1.0f : 0.0f,
        FIXED_DELTA_TIME / TRANSITION_TIME
    );

    struct TutorialDialogStep* step = tutorialCurrentDialog(tutorial);

    int shouldBeTalking = 0;

    if (step) {
        if (tutorial->currentCharacter < tutorial->currentDialogCharacterCount && tutorial->currentTextDelay == 0.0f && tutorial->animationLerp == 1.0f) {
            shouldBeTalking = 1;
        }

        if (controllerGetButtonDown(0, A_BUTTON)) {
            if (tutorial->currentCharacter < tutorial->currentDialogCharacterCount + CHARACTER_ANIMATION_OFFSET) {
                tutorial->currentCharacter = tutorial->currentDialogCharacterCount + CHARACTER_ANIMATION_OFFSET;
            } else {
                ++tutorial->currentDialogIndex;

                if (tutorial->currentDialogIndex == tutorial->currentStep->dialogCount) {
                    tutorialSetNextStep(tutorial, tutorial->currentStep->nextState);
                } else {
                    tutorialInitCurrentMessage(tutorial);
                }
            }
        }

        if (tutorial->currentTextDelay > 0.0f) {
            tutorial->currentTextDelay = mathfMoveTowards(tutorial->currentTextDelay, 0.0f, FIXED_DELTA_TIME);
        } else {
            if (tutorial->textEffects & TutorialPromptEffectInstant) {
                tutorial->currentCharacter = tutorial->currentDialogCharacterCount + CHARACTER_ANIMATION_OFFSET;
            } else {
                tutorial->currentCharacter = mathfMoveTowards(
                    tutorial->currentCharacter, 
                    tutorial->currentDialogCharacterCount + CHARACTER_ANIMATION_OFFSET, 
                    (tutorial->textEffects & TutorialPromptEffectSlow) ?
                        FIXED_DELTA_TIME * SLOW_CHARACTERS_PER_SECOND :
                        FIXED_DELTA_TIME * CHARACTERS_PER_SECOND
                );
            }
        }
    } else if (tutorial->talkingSound != SOUND_ID_NONE) {
        soundPlayerStop(tutorial->talkingSound);
        tutorial->talkingSound = SOUND_ID_NONE;
    }

    if (shouldBeTalking) {
        if (tutorial->talkingSound == SOUND_ID_NONE) {
            tutorial->talkingSound = soundPlayerPlay(SOUNDS_TONYTALKING, 0.5f, 1.0f, NULL);
        } else {
            soundPlayerUpdateVolume(tutorial->talkingSound, 0.5f);
        }
    } else {
        soundPlayerUpdateVolume(tutorial->talkingSound, 0.0f);
    }
    
    return step != NULL;
}

int tutorialIsImmune(struct Tutorial* tutorial) {
    return tutorial->currentStep && tutorial->currentStep->isImmune;
}

struct Coloru8 gDialogBlack = {0, 0, 0, 255};
struct Coloru8 gDialogPurple = {76, 3, 73, 255};

#define SIDE_PADDING    80
#define BOTTOM_PADDING  80
#define IMAGE_HEIGHT    144
#define TEXT_PADDING    16

#define PURPLE_OFFSET_X 16
#define PURPLE_OFFSET_Y 8

#define TEXT_BOX_WIDTH  (SCREEN_WD - 2 * SIDE_PADDING)

#define ANIMATION_OFFSET    15.0f

struct Vector2 gCharacterAnimationDirection[] = {
    {0.5585728148199245f, -0.5421183079065648f},
    {-0.2225213129182435f, -0.0881149789828084f},
    {0.39867296490311777f, 0.15827674457732832f},
    {0.7921854894592923f, 0.9035068117197012f},
    {0.5655626855279585f, 0.8253215223717694f},
    {-0.9278091867685443, 0.4718378770270981},
    {0.6962552387057197, 0.7506643329534035},
    {-0.735979156078419, -0.5423491436650909},
};

void tutorialModifyColor(void* data, int index, char character, int* x, int* y, struct Coloru8* color) {
    struct Tutorial* tutorial = (struct Tutorial*)data;

    float characterLerp = (tutorial->currentCharacter - index) * (1.0f / CHARACTER_ANIMATION_OFFSET);

    if (characterLerp <= 0.0f) {
        color->a = 0;
        return;
    }

    struct TutorialDialogStep* step = tutorialCurrentDialog(tutorial);

    if (step && (step->effects & TutorialPromptEffectShake) != 0) {
        *x += randomInRange(-1, 2);
        *y += randomInRange(-1, 2);
    }


    if (characterLerp >= 1.0f) {
        return;
    }

    struct Vector2* direction = &gCharacterAnimationDirection[(index * 997) & 0x7];

    float animationDir = ((1.0f - characterLerp) * ANIMATION_OFFSET);

    color->a = (unsigned char)(characterLerp * 255.0f);
    *x += (int)(animationDir * direction->x);
    *y += (int)(animationDir * direction->y);
}

u64* gTonyFaces[] = {
    ui_floor_manager_rgba_16b,
    ui_floor_manager_angry_rgba_16b,
    ui_floor_manager_annoyed_rgba_16b,
    ui_floor_manager_confused_rgba_16b,
};

void tutorialRenderTextBacking(struct Tutorial* tutorial, float showAmount, struct RenderState* renderState) {
    int animationOffset = (int)((1.0f - showAmount) * SCREEN_WD);
    int blackX = SIDE_PADDING + animationOffset;
    int blackY = SCREEN_HT - BOTTOM_PADDING - IMAGE_HEIGHT;

    int purpleX = SIDE_PADDING - PURPLE_OFFSET_X - animationOffset;

    int purpleWidth = MIN(TEXT_BOX_WIDTH, blackX - purpleX);

    spriteSetColor(renderState, SOLID_UI_INDEX, gDialogPurple);
    spriteSolid(
        renderState, 
        SOLID_UI_INDEX, 
        purpleX, 
        blackY - PURPLE_OFFSET_Y, 
        purpleWidth, 
        IMAGE_HEIGHT
    );

    if (purpleWidth < TEXT_BOX_WIDTH) {
        spriteSolid(
            renderState, 
            SOLID_UI_INDEX, 
            blackX, 
            blackY - PURPLE_OFFSET_Y, 
            TEXT_BOX_WIDTH - purpleWidth, 
            PURPLE_OFFSET_Y
        );
    }

    spriteSetColor(renderState, SOLID_UI_INDEX, gDialogBlack);
    spriteSolid(
        renderState, 
        SOLID_UI_INDEX, 
        blackX, 
        blackY, 
        TEXT_BOX_WIDTH, 
        IMAGE_HEIGHT
    );

    struct TutorialDialogStep* step = tutorialCurrentDialog(tutorial);

    if (tutorial->animationLerp >= 1.0f && step) {
        int textScale = 0;

        if (step->effects & TutorialPromptEffectScale) {
            textScale = 1;
        }

        fontRenderText(
            renderState, 
            &gNightChilde, 
            step->message, 
            SIDE_PADDING + IMAGE_HEIGHT,
            SCREEN_HT - BOTTOM_PADDING - IMAGE_HEIGHT + TEXT_PADDING, 
            textScale,
            TEXT_BOX_WIDTH - IMAGE_HEIGHT - TEXT_PADDING,
            tutorial,
            tutorialModifyColor
        );

        int imageX = blackX + 8;
        int imageY = blackY + 8;

        spriteCopyImage(renderState, FLOOR_MANAGER_UI_INDEX, gTonyFaces[step->tonyFace], 128, 128, imageX, imageY, 128, 128, 0, 0);
    }
}

#define LONG_SHOT_PROMPT        200

void tutorialModifyActionPrompt(void* data, int index, char character, int* x, int* y, struct Coloru8* color) {
    struct Tutorial* tutorial = (struct Tutorial*)data;

    struct Vector2* direction = &gCharacterAnimationDirection[(index * 997) & 0x7];

    float animationDir = ((1.0f - tutorial->promptBoxLerp) * LONG_SHOT_PROMPT);

    color->a = (unsigned char)(tutorial->promptBoxLerp * 255.0f);
    *x += (int)(animationDir * direction->x);
    *y += (int)(animationDir * direction->y);
}

#define BUTTON_SIDE_PADDING    80
#define BUTTON_BUTTON_PADDING  80
#define BUTTOM_IMAGE_HEIGHT    48
#define BUTTON_TEXT_PADDING    16

#define BUTTON_TEXT_BOX_WIDTH  128

char* gPromptText[] = {
    "",
    "Pickup",
    "Drop"
};

struct SpriteTile gButtonSprites[] = {
    {0, 0, 0, 0},
    {0, 0, 32, 32},
    {32, 0, 32, 32},
};

void tutorialRenderButtonPrompt(struct Tutorial* tutorial, float showAmount, char* message, struct SpriteTile* spriteTile, struct RenderState* renderState) {
    int animationOffset = (int)((1.0f - showAmount) * SCREEN_WD);
    int blackX = BUTTON_SIDE_PADDING + animationOffset;
    int blackY = SCREEN_HT - BUTTON_BUTTON_PADDING - BUTTOM_IMAGE_HEIGHT;

    int purpleX = BUTTON_SIDE_PADDING - PURPLE_OFFSET_X - animationOffset;

    int purpleWidth = MIN(BUTTON_TEXT_BOX_WIDTH, blackX - purpleX);

    spriteSetColor(renderState, SOLID_UI_INDEX, gDialogPurple);
    spriteSolid(
        renderState, 
        SOLID_UI_INDEX, 
        purpleX, 
        blackY - PURPLE_OFFSET_Y, 
        purpleWidth, 
        BUTTOM_IMAGE_HEIGHT
    );

    if (purpleWidth < BUTTON_TEXT_BOX_WIDTH) {
        spriteSolid(
            renderState, 
            SOLID_UI_INDEX, 
            blackX, 
            blackY - PURPLE_OFFSET_Y, 
            BUTTON_TEXT_BOX_WIDTH - purpleWidth, 
            PURPLE_OFFSET_Y
        );
    }

    spriteSetColor(renderState, SOLID_UI_INDEX, gDialogBlack);
    spriteSolid(
        renderState, 
        SOLID_UI_INDEX, 
        blackX, 
        blackY, 
        BUTTON_TEXT_BOX_WIDTH, 
        BUTTOM_IMAGE_HEIGHT
    );

    fontRenderText(
        renderState, 
        &gNightChilde, 
        message, 
        BUTTON_SIDE_PADDING + BUTTON_TEXT_PADDING,
        SCREEN_HT - BUTTON_BUTTON_PADDING - BUTTOM_IMAGE_HEIGHT + BUTTON_TEXT_PADDING, 
        0,
        0,
        tutorial,
        tutorialModifyActionPrompt
    );

    struct Coloru8 buttonColor = {255, 255, 255, 255};
    buttonColor.a = (int)(255.0f * showAmount);

    spriteSetColor(renderState, BUTTONS_UI_INDEX, buttonColor);
    spriteDrawTile(
        renderState, 
        BUTTONS_UI_INDEX, 
        BUTTON_SIDE_PADDING + BUTTON_TEXT_BOX_WIDTH - spriteTile->w - 8,
        SCREEN_HT - BUTTON_BUTTON_PADDING - BUTTOM_IMAGE_HEIGHT + 8,
        spriteTile->w, spriteTile->h,
        *spriteTile
    );
}

void tutorialRender(struct Tutorial* tutorial, struct RenderState* renderState) {
    if (tutorial->animationLerp) {
        tutorialRenderTextBacking(tutorial, tutorial->animationLerp, renderState);
    }

    if (tutorial->promptBoxLerp && tutorial->currentPrompt) {
        tutorialRenderButtonPrompt(
            tutorial, 
            tutorial->promptBoxLerp, 
            gPromptText[tutorial->currentPrompt], 
            &gButtonSprites[tutorial->currentPrompt], 
            renderState
        );
    }
}
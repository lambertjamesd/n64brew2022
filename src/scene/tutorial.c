#include "tutorial.h"

#include "../ui/nightchilde.h"

#include "../build/assets/materials/ui.h"
#include "../graphics/graphics.h"
#include "../util/time.h"
#include "../math/vector2.h"
#include "../math/mathf.h"
#include "../controls/controller.h"

#include <string.h>

#define TRANSITION_TIME 0.5f

#define CHARACTERS_PER_SECOND   10.0f

#define CHARACTER_ANIMATION_OFFSET    4.0f

#define SHAKE_PERIOD    0.1f

void tutorialInitCurrentState(struct Tutorial* tutorial);

struct TutorialScriptStep gIntroScript[] = {
    {
        "Welcome to your first day of\norientation\n" 
        "I am Tony Spook\n"
        "Your floor manager"
    },
    {
        "Your job is to fulfill orders"
    },
    {
        "Products come in on the conveyor\n" 
        "Belts and you carry them to\n"
        "the portals"
    },
};

struct TutorialScriptStep gPortalPrompt[] = {
    {
        "Great\n"
        "Now place the pumpkin in\n"
        "the portal"
    }
};

struct TutorialScriptStep gWrongDrop[] = {
    {
        "No\n"
        "You need to drop the pumpkin\n"
        "in the the portal"
    }
};

struct TutorialScriptStep gTables[] = {
    {
        "Excellent work\n"
        "Now the shipments coming in\n"
        "wont always match the requests\n"
        "that come from the portals"
    },
    {
        "You can store objects on the\n"
        "tables that are not needed\n"
        "right away"
    }
};

struct TutorialScriptStep gCompleteShift[] = {
    {
        "Thats all you need to know\n"
        "Complete your shift today\n"
        "tomorrow we will assign you to\n"
        "a larger warehouse"
    },
    {
        "Oh and do not drop too many\n"
        "items on the floor or fail\n"
        "to fulfill requests\n"
        "if you do"
    },
    {
        "He will come"
    },
    {
        "and if he catches you\n"
        "you will be..."
    },
    {
        "terminated"
    },
    {
        "..."
    },
    {
        "have a nice shift"
    },
};

struct TutorialScript gTutorialScripts[TutorialStateCount] = {
    [TutorialStateIntro] = {
        gIntroScript,
        sizeof(gIntroScript) / sizeof(gIntroScript[0]),
        TutorialStatePickup,
    },
    [TutorialStatePortalDialog] = {
        gPortalPrompt,
        sizeof(gPortalPrompt) / sizeof(gPortalPrompt[0]),
        TutorialStateDrop,
    },
    [TutorialStateWrongDrop] = {
        gWrongDrop,
        sizeof(gWrongDrop) / sizeof(gWrongDrop[0]),
        TutorialStatePickup,
    },
    [TutorialStateTable] = {
        gTables,
        sizeof(gTables) / sizeof(gTables[0]),
        TutorialStateSecondDrop,
    },
    [TutorialStatePlay] = {
        gCompleteShift,
        sizeof(gCompleteShift) / sizeof(gCompleteShift[0]),
        TutorialStateWait,
    },
};

void tutorialInit(struct Tutorial* tutorial) {
    tutorial->state = TutorialStateWait;
    tutorial->nextState = TutorialStateWait;
    tutorial->animationLerp = 0.0f;

    tutorialSetNextState(tutorial, TutorialStateIntro);
}

void tutorialSetNextState(struct Tutorial* tutorial, enum TutorialState state) {
    if (tutorial->state == TutorialStateWait) {
        tutorial->state = state;
        tutorialInitCurrentState(tutorial);
    }
    
    tutorial->nextState = state;
}

void tutorialItemPickedUp(struct Tutorial* tutorial) {
    if (tutorial->state == TutorialStatePickup) {
        tutorial->nextState = TutorialStatePortalDialog;
    }
}

void tutorialItemDropped(struct Tutorial* tutorial, int success) {
    if (tutorial->state == TutorialStateDrop) {
        if (success) {
            tutorial->nextState = TutorialStateTable;
        } else {
            tutorial->nextState = TutorialStateWrongDrop;
        }
    }

    if (tutorial->state == TutorialStateSecondDrop && success) {
        tutorial->nextState = TutorialStatePlay;
    }
}

void tutorialItemTabled(struct Tutorial* tutorial) {

}

void tutorialInitCurrentMessage(struct Tutorial* tutorial) {
    if (tutorial->currentScript) {
        tutorial->currentStepCharacterCount = strlen(tutorial->currentScript->steps[tutorial->currentStep].message);
    } else {
        tutorial->currentStepCharacterCount = 0.0f;
    }

    tutorial->currentCharacter = 0.0f;
}

void tutorialInitCurrentState(struct Tutorial* tutorial) {
    tutorial->currentScript = &gTutorialScripts[tutorial->state];
    tutorial->currentStep = 0;

    if (!tutorial->currentScript->steps) {
        tutorial->currentScript = NULL;
    }

    tutorialInitCurrentMessage(tutorial);
}

struct TutorialScriptStep* tutorialCurrentStep(struct Tutorial* tutorial) {
    if (!tutorial->currentScript) {
        return NULL;
    }

    if (tutorial->currentStep >= tutorial->currentScript->count) {
        return NULL;
    }

    return &tutorial->currentScript->steps[tutorial->currentStep];
}

int tutorialUpdate(struct Tutorial* tutorial) {
    if (tutorial->state == TutorialStateWait) {
        return 0;
    }

    if (tutorial->state != tutorial->nextState) {
        tutorial->animationLerp -= (FIXED_DELTA_TIME / TRANSITION_TIME);

        if (tutorial->animationLerp < 0.0f) {
            tutorial->animationLerp = 0.0f;
            tutorial->state = tutorial->nextState;

            tutorialInitCurrentState(tutorial);
        }
    } else if (tutorial->animationLerp < 1.0f) {
        tutorial->animationLerp += (FIXED_DELTA_TIME / TRANSITION_TIME);

        if (tutorial->animationLerp > 1.0f) {
            tutorial->animationLerp = 1.0f;
        }
    }

    if (tutorial->animationLerp == 1.0f) {
        if (tutorial->currentCharacter < tutorial->currentStepCharacterCount + CHARACTER_ANIMATION_OFFSET) {
            tutorial->currentCharacter += FIXED_DELTA_TIME * CHARACTERS_PER_SECOND;

            if (tutorial->currentCharacter > tutorial->currentStepCharacterCount + CHARACTER_ANIMATION_OFFSET) {
                tutorial->currentCharacter = tutorial->currentStepCharacterCount + CHARACTER_ANIMATION_OFFSET;
            }
        }
    }

    struct TutorialScriptStep* step = tutorialCurrentStep(tutorial);

    if (step) {
        if (controllerGetButtonDown(0, A_BUTTON)) {
            if (tutorial->currentCharacter < tutorial->currentStepCharacterCount + CHARACTER_ANIMATION_OFFSET) {
                tutorial->currentCharacter = tutorial->currentStepCharacterCount + CHARACTER_ANIMATION_OFFSET;
            } else {
                ++tutorial->currentStep;

                if (tutorial->currentStep == tutorial->currentScript->count) {
                    tutorialSetNextState(tutorial, tutorial->currentScript->nextState);
                } else {
                    tutorialInitCurrentMessage(tutorial);
                }
            }
        }
    }
    
    return tutorial->state != TutorialStatePickup && 
        tutorial->state != TutorialStateDrop &&
        tutorial->state != TutorialStateSecondDrop;
}

struct Coloru8 gDialogBack = {0, 0, 0, 255};
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

    // *x += randomInRange(-1, 2);
    // *y += randomInRange(-1, 2);

    if (characterLerp >= 1.0f) {
        return;
    }

    struct Vector2* direction = &gCharacterAnimationDirection[(index * 997) & 0x7];

    float animationDir = ((1.0f - characterLerp) * ANIMATION_OFFSET);

    color->a = (unsigned char)(characterLerp * 255.0f);
    *x += (int)(animationDir * direction->x);
    *y += (int)(animationDir * direction->y);
}

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

    spriteSetColor(renderState, SOLID_UI_INDEX, gDialogBack);
    spriteSolid(
        renderState, 
        SOLID_UI_INDEX, 
        blackX, 
        blackY, 
        TEXT_BOX_WIDTH, 
        IMAGE_HEIGHT
    );

    struct TutorialScriptStep* step = tutorialCurrentStep(tutorial);

    if (tutorial->animationLerp >= 1.0f && step) {
        fontRenderText(
            renderState, 
            &gNightChilde, 
            step->message, 
            SIDE_PADDING + IMAGE_HEIGHT,
            SCREEN_HT - BOTTOM_PADDING - IMAGE_HEIGHT + TEXT_PADDING, 
            0,
            tutorial,
            tutorialModifyColor
        );

        int imageX = blackX + 8;
        int imageY = blackY + 8;

        spriteDraw(renderState, FLOOR_MANAGER_UI_00_INDEX, imageX, imageY, 64, 32, 0, 0, 0, 0);
        spriteDraw(renderState, FLOOR_MANAGER_UI_10_INDEX, imageX + 64, imageY, 64, 32, 0, 0, 0, 0);

        spriteDraw(renderState, FLOOR_MANAGER_UI_01_INDEX, imageX, imageY + 32, 64, 32, 0, 0, 0, 0);
        spriteDraw(renderState, FLOOR_MANAGER_UI_11_INDEX, imageX + 64, imageY + 32, 64, 32, 0, 0, 0, 0);

        spriteDraw(renderState, FLOOR_MANAGER_UI_02_INDEX, imageX, imageY + 64, 64, 32, 0, 0, 0, 0);
        spriteDraw(renderState, FLOOR_MANAGER_UI_12_INDEX, imageX + 64, imageY + 64, 64, 32, 0, 0, 0, 0);

        spriteDraw(renderState, FLOOR_MANAGER_UI_03_INDEX, imageX, imageY + 96, 64, 32, 0, 0, 0, 0);
        spriteDraw(renderState, FLOOR_MANAGER_UI_13_INDEX, imageX + 64, imageY + 96, 64, 32, 0, 0, 0, 0);
    }
}

#define LONG_SHOT_PROMPT        200

void tutorialModifyActionPrompt(void* data, int index, char character, int* x, int* y, struct Coloru8* color) {
    struct Tutorial* tutorial = (struct Tutorial*)data;

    struct Vector2* direction = &gCharacterAnimationDirection[(index * 997) & 0x7];

    float animationDir = ((1.0f - tutorial->animationLerp) * LONG_SHOT_PROMPT);

    color->a = (unsigned char)(tutorial->animationLerp * 255.0f);
    *x += (int)(animationDir * direction->x);
    *y += (int)(animationDir * direction->y);
}

#define BUTTON_SIDE_PADDING    80
#define BUTTON_BUTTON_PADDING  80
#define BUTTOM_IMAGE_HEIGHT    48
#define BUTTON_TEXT_PADDING    16

#define BUTTON_TEXT_BOX_WIDTH  128

struct SpriteTile gButtonSprites[] = {
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

    spriteSetColor(renderState, SOLID_UI_INDEX, gDialogBack);
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
        tutorial,
        tutorialModifyActionPrompt
    );

    struct Coloru8 buttonColor = {255, 255, 255, 255};
    buttonColor.a = (int)(255.0f * tutorial->animationLerp);

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
    if (tutorial->state == TutorialStateWait) {
        return;
    }
    
    if (tutorial->currentScript) {
        tutorialRenderTextBacking(tutorial, tutorial->animationLerp, renderState);
    }

    if (tutorial->state == TutorialStatePickup) {
        tutorialRenderButtonPrompt(tutorial, tutorial->animationLerp, "Pickup", &gButtonSprites[0], renderState);
    }

    if (tutorial->state == TutorialStateDrop) {
        tutorialRenderButtonPrompt(tutorial, tutorial->animationLerp, "Drop", &gButtonSprites[1], renderState);
    }
}
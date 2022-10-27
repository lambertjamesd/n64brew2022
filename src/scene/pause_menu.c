#include "pause_menu.h"

#include "../build/assets/materials/ui.h"

#include "../controls/controller.h"
#include "../math/mathf.h"
#include "../util/time.h"
#include "../defs.h"
#include "../graphics/graphics.h"
#include "../ui/spritefont.h"
#include "../ui/nightchilde.h"
#include "../level/level.h"

#define ANIMATION_TIME  0.5f

void pauseMenuInit(struct PauseMenu* pauseMenu) {
    pauseMenu->isEnabled = 0;
    pauseMenu->selectedItem = 0;
    pauseMenu->showAnimation = 0.0f;
}

int pauseMenuUpdate(struct PauseMenu* pauseMenu) {
    if (controllerGetButtonDown(0, START_BUTTON)) {
        pauseMenu->isEnabled = !pauseMenu->isEnabled;
    }

    pauseMenu->showAnimation = mathfMoveTowards(pauseMenu->showAnimation, pauseMenu->isEnabled ? 1.0f : 0.0f, FIXED_DELTA_TIME / ANIMATION_TIME);

    if (pauseMenu->showAnimation >= 1.0f) {
        if (controllerGetDirectionDown(0) & (ControllerDirectionDown | ControllerDirectionUp)) {
            pauseMenu->selectedItem = 1 - pauseMenu->selectedItem;
        }

        if (controllerGetButtonDown(0, A_BUTTON)) {
            if (pauseMenu->selectedItem == 0) {
                pauseMenu->isEnabled = 0;
            } else {
                levelQueueLoad(MAIN_MENU_LEVEL);
            }
        }
    }

    return pauseMenu->showAnimation > 0.0f;
}

#define SIDE_PADDING    160
#define BOTTOM_PADDING  140
#define IMAGE_HEIGHT    200
#define TEXT_PADDING    16

#define PURPLE_OFFSET_X 16
#define PURPLE_OFFSET_Y 8

#define TEXT_BOX_WIDTH  (SCREEN_WD - 2 * SIDE_PADDING)

extern struct Coloru8 gDialogBlack;
extern struct Coloru8 gDialogPurple;
struct Coloru8 gColorGreen = {106, 192, 35, 255};

void pauseMenuRender(struct PauseMenu* pauseMenu, struct RenderState* renderState) {
    if (pauseMenu->showAnimation == 0.0f) {
        return;
    }

    int animationOffset = (int)((1.0f - pauseMenu->showAnimation) * SCREEN_WD);
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

    if (pauseMenu->showAnimation < 1.0f) {
        return;
    }

    fontRenderText(
        renderState,
        &gNightChilde,
        "Paused",
        320 - 130,
        blackY + 20,
        2,
        NULL,
        NULL
    );

    spriteSetColor(renderState, NIGHTCHILDE_INDEX, pauseMenu->selectedItem == 0 ? gColorGreen : gColorWhite);
    fontRenderText(
        renderState,
        &gNightChilde,
        "Resume",
        320 - 100,
        blackY + 100,
        1,
        NULL,
        NULL
    );

    spriteSetColor(renderState, NIGHTCHILDE_INDEX, pauseMenu->selectedItem == 1 ? gColorGreen : gColorWhite);
    fontRenderText(
        renderState,
        &gNightChilde,
        "Main Menu",
        320 - 100,
        blackY + 150,
        1,
        NULL,
        NULL
    );
}

int pauseMenuIsPaused(struct PauseMenu* pauseMenu) {
    return pauseMenu->showAnimation > 0.0f;
}
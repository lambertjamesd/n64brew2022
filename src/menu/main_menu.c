#include "main_menu.h"

#include "../graphics/image.h"

#include "../build/assets/materials/ui.h"
#include "../level/level.h"
#include "../math/mathf.h"
#include "../util/time.h"
#include "../controls/controller.h"
#include "./ui.h"
#include "../ui/spritefont.h"
#include "../ui/nightchilde.h"
#include "../savefile/savefile.h"

#define FADE_TIME       0.5f

#define MENU_OPEN_TIME  0.5f

#define PRESS_START_DELAY       2.0f
#define PRESS_START_INTRO_TIME  0.5f

struct MainMenu gMainMenu;

void mainMenuInit(struct MainMenu* mainMenu) {
    mainMenu->currentState = MainMenuTitleScreen;
    mainMenu->fadeAmount = 0.0f;
    mainMenu->titleAnimation = 0.0f;
    mainMenu->windowOpenAnimation = 0.0f;
    mainMenu->levelToLoad = NO_QUEUED_LEVEL;
    mainMenu->selectedLevel = 0;
}

int mainMenuCanPlayerLevel(struct MainMenu* mainMenu, int levelIndex) {
    return levelIndex == 0 || saveFileIsLevelComplete(levelIndex - 1);
}

void mainMenuUpdate(struct MainMenu* mainMenu) {
    if (mainMenu->levelToLoad == NO_QUEUED_LEVEL) {
        mainMenu->fadeAmount = mathfMoveTowards(mainMenu->fadeAmount, 1.0f, FIXED_DELTA_TIME * (1.0f / FADE_TIME));
    } else {
        mainMenu->fadeAmount = mathfMoveTowards(mainMenu->fadeAmount, 0.0f, FIXED_DELTA_TIME * (1.0f / FADE_TIME));

        if (mainMenu->fadeAmount == 0.0f) {
            levelQueueLoad(mainMenu->levelToLoad);
        }
    }

    mainMenu->windowOpenAnimation = mathfMoveTowards(
        mainMenu->windowOpenAnimation,
        mainMenu->currentState == MainMenuLevelList ? 1.0f : 0.0f,
        FIXED_DELTA_TIME * (1.0f / MENU_OPEN_TIME)
    );

    mainMenu->titleAnimation = mathfMoveTowards(
        mainMenu->titleAnimation,
        mainMenu->currentState == MainMenuTitleScreen ? PRESS_START_DELAY + PRESS_START_INTRO_TIME : 0.0f,
        FIXED_DELTA_TIME
    );

    if (mainMenu->currentState == MainMenuTitleScreen) {
        if (controllerGetButtonDown(0, START_BUTTON) && !controllerGetButtonDown(0, A_BUTTON)) {
            mainMenu->currentState = MainMenuLevelList;
        }
    }

    if (mainMenu->currentState == MainMenuLevelList) {
        if (controllerGetButtonDown(0, A_BUTTON) && mainMenuCanPlayerLevel(mainMenu, mainMenu->selectedLevel)) {
            mainMenu->levelToLoad = mainMenu->selectedLevel;
            mainMenu->currentState = MainMenuLoading;
        }

        if (controllerGetButtonDown(0, B_BUTTON)) {
            mainMenu->currentState = MainMenuTitleScreen;
        }
    }

    if (mainMenu->windowOpenAnimation == 1.0f) {
        if (controllerGetDirectionDown(0) & ControllerDirectionRight) {
            if (mainMenu->selectedLevel + 1 < levelGetCount()) {
                ++mainMenu->selectedLevel;
            }
        }

        if (controllerGetDirectionDown(0) & ControllerDirectionLeft) {
            if (mainMenu->selectedLevel > 0) {
                --mainMenu->selectedLevel;
            }
        }
    }
}

#define LEVEL_LIST_WIDTH    596
#define LEVEL_LIST_HEIGHT   452

struct Coloru8 gTextDrop = {37, 37, 37, 255};
struct Coloru8 gPurpleColor = {157, 79, 221, 255};

struct Coloru8 gDisabledColor = {190, 170, 200, 255};
struct Coloru8 gRedColor = {255, 30, 30, 255};

void mainMenuRender(struct MainMenu* mainMenu, struct RenderState* renderState, struct GraphicsTask* graphicsTask) {
    uiInitSpirtes(renderState);

    graphicsCopyImage(renderState, ui_title_screen_rgba_16b, SCREEN_WD, SCREEN_HT, 0, 0, 0, 0, SCREEN_WD, SCREEN_HT, gColorWhite);

    if (mainMenu->windowOpenAnimation) {
        graphicsCopyImage(
            renderState, 
            ui_level_list_rgba_16b, 
            LEVEL_LIST_WIDTH, LEVEL_LIST_HEIGHT, 
            0, 0, 
            22 + (int)((1.0f - mainMenu->windowOpenAnimation) * (SCREEN_WD / 2 - 22)), 
            14 + (int)((1.0f - mainMenu->windowOpenAnimation) * (SCREEN_HT - 14)), 
            (int)(596 * mainMenu->windowOpenAnimation), (int)(452 * mainMenu->windowOpenAnimation), 
        gColorWhite);
    }

    spriteInit(renderState);

    if (mainMenu->windowOpenAnimation == 1.0f) {
        struct LevelMetadata* levelMetadata = levelGetMetadata(mainMenu->selectedLevel);

        int isEnabled = mainMenuCanPlayerLevel(mainMenu, mainMenu->selectedLevel);

        spriteSetColor(renderState, NIGHTCHILDE_INDEX, gTextDrop);
        fontRenderText(renderState, &gNightChilde, levelMetadata->name, 388, 182, 1, NULL, NULL);
        spriteSetColor(renderState, NIGHTCHILDE_INDEX, isEnabled ? gPurpleColor : gDisabledColor);
        fontRenderText(renderState, &gNightChilde, levelMetadata->name, 390, 180, 1, NULL, NULL);

        if (!isEnabled) {
            spriteSetColor(renderState, NIGHTCHILDE_INDEX, gRedColor);
            fontRenderText(renderState, &gNightChilde, "out of stock", 390, 224, 0, NULL, NULL);
        }
    }

    float lerpAmount = 1.0f - (mainMenu->titleAnimation - PRESS_START_DELAY) * (1.0f / PRESS_START_INTRO_TIME);

    if (lerpAmount < 1.0f) {
        int y = 260 + (int)(lerpAmount * lerpAmount * 400);
        spriteSetColor(renderState, NIGHTCHILDE_INDEX, gColorWhite);
        fontRenderText(renderState, &gNightChilde, "press start", 198, y + 2, 1, NULL, NULL);
        spriteSetColor(renderState, NIGHTCHILDE_INDEX, gColorBlack);
        fontRenderText(renderState, &gNightChilde, "press start", 200, y, 1, NULL, NULL);
    }

    spriteFinish(renderState);

    if (mainMenu->fadeAmount < 1.0f) {
        gDPPipeSync(renderState->dl++);
        gDPSetCombineLERP(renderState->dl++, 0, 0, 0, ENVIRONMENT, 0, 0, 0, ENVIRONMENT, 0, 0, 0, ENVIRONMENT, 0, 0, 0, ENVIRONMENT);
        gDPSetRenderMode(renderState->dl++, G_RM_XLU_SURF, G_RM_XLU_SURF2);
        gDPSetEnvColor(renderState->dl++, 0, 0, 0, 255 - (int)((255.0f) * mainMenu->fadeAmount));
        gDPFillRectangle(renderState->dl++, 0, 0, SCREEN_WD, SCREEN_HT);
    }
}

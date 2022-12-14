#include "main_menu.h"

#include "../graphics/image.h"

#include "../build/assets/materials/ui.h"
#include "../build/src/audio/clips.h"
#include "../level/level.h"
#include "../math/mathf.h"
#include "../util/time.h"
#include "../controls/controller.h"
#include "./ui.h"
#include "../ui/spritefont.h"
#include "../ui/nightchilde.h"
#include "../savefile/savefile.h"
#include "../audio/audio.h"
#include "../audio/soundplayer.h"


#define FADE_TIME       0.5f

#define MENU_OPEN_TIME  0.5f

#define PRESS_START_DELAY       2.0f
#define PRESS_START_INTRO_TIME  0.5f

#define LIFE_INSURCE_PERCENT    8
#define _401K_PERCENT           10

struct MainMenu gMainMenu;

void mainMenuInit(struct MainMenu* mainMenu) {
    mainMenu->currentState = MainMenuTitleScreen;
    mainMenu->fadeAmount = 0.0f;
    mainMenu->titleAnimation = 0.0f;
    mainMenu->windowOpenAnimation = 0.0f;
    mainMenu->levelToLoad = NO_QUEUED_LEVEL;
    mainMenu->selectedLevel = 0;
}

void mainMenuEnter(struct MainMenu* mainMenu) {
    if (mainMenu->currentState == MainMenuLoading) {
        mainMenu->currentState = MainMenuLevelList;
    }

    mainMenu->fadeAmount = 0.0f;
    mainMenu->titleAnimation = 0.0f;
    mainMenu->windowOpenAnimation = 0.0f;
    mainMenu->levelToLoad = NO_QUEUED_LEVEL;

    mainMenu->musicSound = SOUND_ID_NONE;
}

void mainMenuShowCredits(struct MainMenu* mainMenu) {
    mainMenu->currentState = MainMenuCredits;
    levelQueueLoad(MAIN_MENU_LEVEL);
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

    if (!soundPlayerIsPlaying(mainMenu->musicSound)) {
        mainMenu->musicSound = soundPlayerPlay(SOUNDS_N64_GONE_AWAY_FINAL, 1.0f, 0.5f, NULL);
    }

    mainMenu->windowOpenAnimation = mathfMoveTowards(
        mainMenu->windowOpenAnimation,
        (mainMenu->currentState == MainMenuLevelList || mainMenu->currentState == MainMenuCredits) ? 1.0f : 0.0f,
        FIXED_DELTA_TIME * (1.0f / MENU_OPEN_TIME)
    );

    mainMenu->titleAnimation = mathfMoveTowards(
        mainMenu->titleAnimation,
        mainMenu->currentState == MainMenuTitleScreen ? PRESS_START_DELAY + PRESS_START_INTRO_TIME : 0.0f,
        FIXED_DELTA_TIME
    );

    if (mainMenu->currentState == MainMenuTitleScreen) {
        if (controllerGetButtonDown(0, START_BUTTON) && !controllerGetButtonDown(0, A_BUTTON)) {
            if (controllerGetButton(0, R_TRIG | Z_TRIG)) {
                saveFileErase();
                soundPlayerPlay(SOUNDS_TRASHITEM, 1.0f, 1.0f, NULL);
            } else {
                mainMenu->currentState = MainMenuLevelList;
                soundPlayerPlay(SOUNDS_BONK, 1.0f, 1.0f, NULL);
            }
        }
    } else if (mainMenu->currentState == MainMenuLevelList) {
        if (controllerGetButtonDown(0, A_BUTTON) && mainMenuCanPlayerLevel(mainMenu, mainMenu->selectedLevel)) {
            mainMenu->levelToLoad = mainMenu->selectedLevel;
            mainMenu->currentState = MainMenuLoading;
            soundPlayerPlay(SOUNDS_BONK, 1.0f, 1.0f, NULL);
        }

        if (controllerGetButtonDown(0, B_BUTTON)) {
            mainMenu->currentState = MainMenuTitleScreen;
            soundPlayerPlay(SOUNDS_BONK, 1.0f, 1.0f, NULL);
        }
    } else if (mainMenu->currentState == MainMenuCredits) {
        if (controllerGetButtonDown(0, START_BUTTON)) {
            mainMenu->currentState = MainMenuTitleScreen;
            soundPlayerPlay(SOUNDS_BONK, 1.0f, 1.0f, NULL);
        }
    }

    if (mainMenu->windowOpenAnimation == 1.0f) {
        if (controllerGetDirectionDown(0) & ControllerDirectionRight) {
            if (mainMenu->selectedLevel + 1 < levelGetCount()) {
                ++mainMenu->selectedLevel;
                soundPlayerPlay(SOUNDS_BONK, 1.0f, 1.0f, NULL);
            }
        }

        if (controllerGetDirectionDown(0) & ControllerDirectionLeft) {
            if (mainMenu->selectedLevel > 0) {
                --mainMenu->selectedLevel;
                soundPlayerPlay(SOUNDS_BONK, 1.0f, 1.0f, NULL);
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

void* gLevelImages[] = {
    ui_tutorial_thumb_rgba_16b,
    ui_level_02_thumb_rgba_16b,
    ui_level_03_thumb_rgba_16b,
    ui_level_04_thumb_rgba_16b,
    ui_level_05_thumb_rgba_16b,
};

void formatTime(short tenthSeconds, char* output) {
    int tenths = tenthSeconds;
    int seconds = tenths / 10;
    tenths -= seconds * 10;
    int minutes = seconds / 60;
    seconds -= minutes * 60;
    int hours = minutes / 60;
    minutes -= hours * 60;

    sprintf(output, "%d:%02d:%02d.%01d", hours, minutes, seconds, tenths);
}

void formatCents(short cents, char* output) {
    int dollars = cents / 100;
    cents -= dollars * 100;

    sprintf(output, "$%d.%02d", dollars, cents);
}

short mainMenuTotalTime(struct MainMenu* mainMenu) {
    short result = 0;

    for (int i = 0; i < 5; ++i) {
        if (!saveFileIsLevelComplete(i)) {
            return 0;
        }

        result += saveFileLevelTime(i);
    }

    return result;
}

void mainMenuRender(struct MainMenu* mainMenu, struct RenderState* renderState, struct GraphicsTask* graphicsTask) {
    uiInitSpirtes(renderState);

    graphicsCopyImage(renderState, ui_title_screen_rgba_16b, SCREEN_WD, SCREEN_HT, 0, 0, 0, 0, SCREEN_WD, SCREEN_HT, gColorWhite);

    if (mainMenu->windowOpenAnimation) {
        int imageWidth = LEVEL_LIST_WIDTH;
        int imageHeight = LEVEL_LIST_HEIGHT;

        void* imageSource = ui_level_list_rgba_16b;

        if (mainMenu->currentState == MainMenuCredits) {
            imageWidth = 640;
            imageHeight = 480;
            imageSource = ui_credits_rgba_16b;
        }

        int x = (SCREEN_WD - imageWidth) >> 1;
        int y = (SCREEN_HT - imageHeight) >> 1;
        
        graphicsCopyImage(
            renderState, 
            imageSource, 
            imageWidth, imageHeight, 
            0, 0, 
            x + (int)((1.0f - mainMenu->windowOpenAnimation) * (SCREEN_WD / 2 - x)), 
            y + (int)((1.0f - mainMenu->windowOpenAnimation) * (SCREEN_HT - y)), 
            (int)(imageWidth * mainMenu->windowOpenAnimation), (int)(imageHeight * mainMenu->windowOpenAnimation), 
        gColorWhite);
    }

    spriteInit(renderState);

    if (mainMenu->windowOpenAnimation == 1.0f) {
        if (mainMenu->currentState == MainMenuLevelList) {
            struct LevelMetadata* levelMetadata = levelGetMetadata(mainMenu->selectedLevel);

            int isEnabled = mainMenuCanPlayerLevel(mainMenu, mainMenu->selectedLevel);

            spriteSetColor(renderState, NIGHTCHILDE_INDEX, gTextDrop);
            fontRenderText(renderState, &gNightChilde, levelMetadata->name, 388, 182, 1, 0, NULL, NULL);
            spriteSetColor(renderState, NIGHTCHILDE_INDEX, isEnabled ? gPurpleColor : gDisabledColor);
            fontRenderText(renderState, &gNightChilde, levelMetadata->name, 390, 180, 1, 0, NULL, NULL);

            if (saveFileIsLevelComplete(mainMenu->selectedLevel)) {
                char timeAsString[12];
                formatTime(saveFileLevelTime(mainMenu->selectedLevel), timeAsString);
                spriteSetColor(renderState, NIGHTCHILDE_INDEX, gTextDrop);
                fontRenderText(
                    renderState, 
                    &gNightChilde, 
                    timeAsString, 
                    590 - fontMeasure(&gNightChilde, timeAsString, 0, 0), 
                    224, 
                    0, 
                    0,
                    NULL, 
                    NULL
                );
            }

            if (!isEnabled) {
                spriteSetColor(renderState, NIGHTCHILDE_INDEX, gRedColor);
                fontRenderText(renderState, &gNightChilde, "out of stock", 390, 224, 0, 0, NULL, NULL);
            }

            if (gLevelImages[mainMenu->selectedLevel]) {
                spriteCopyImage(renderState, TUTORIAL_THUMB_INDEX, gLevelImages[mainMenu->selectedLevel], 320, 237, 54, 127, 320, 237, 0, 0);
            }

            spriteSetColor(renderState, SOLID_UI_INDEX, gPurpleColor);
            spriteSolid(renderState, SOLID_UI_INDEX, 189 + 22 + 53 * mainMenu->selectedLevel, 383 + 14, 41, 36);
        } else if (mainMenu->currentState == MainMenuCredits) {
            char timeAsString[16];
            short time = mainMenuTotalTime(mainMenu);
            if (time) {
                formatTime(time, timeAsString);
            } else {
                timeAsString[0] = 'n';
                timeAsString[1] = 'a';
                timeAsString[2] = 0;
            }
            spriteSetColor(renderState, NIGHTCHILDE_INDEX, gPurpleColor);
            fontRenderText(
                renderState, 
                &gNightChilde, 
                timeAsString, 
                40, 
                78, 
                0, 
                0,
                NULL, 
                NULL
            );

            fontRenderText(
                renderState, 
                &gNightChilde, 
                "$15 H", 
                150, 
                78, 
                0, 
                0,
                NULL, 
                NULL
            );

            int cents = time * 15 * 100 / (10 * 60 * 60);
            formatCents(cents, timeAsString);

            fontRenderText(
                renderState, 
                &gNightChilde, 
                timeAsString, 
                236, 
                78, 
                0, 
                0,
                NULL, 
                NULL
            );

            int lifeInsurance = (cents * LIFE_INSURCE_PERCENT) / 100;
            int _401k = (cents * _401K_PERCENT) / 100;

            formatCents(lifeInsurance, timeAsString);

            fontRenderText(
                renderState, 
                &gNightChilde, 
                timeAsString, 
                514, 
                75, 
                0, 
                0,
                NULL, 
                NULL
            );

            formatCents(_401k, timeAsString);

            fontRenderText(
                renderState, 
                &gNightChilde, 
                timeAsString, 
                514, 
                123, 
                0, 
                0,
                NULL, 
                NULL
            );

            formatCents(cents - lifeInsurance - _401k, timeAsString);

            fontRenderText(
                renderState, 
                &gNightChilde, 
                timeAsString, 
                295 - fontMeasure(&gNightChilde, timeAsString, 0, 0), 
                354, 
                1, 
                0,
                NULL, 
                NULL
            );
        }
    }

    float lerpAmount = 1.0f - (mainMenu->titleAnimation - PRESS_START_DELAY) * (1.0f / PRESS_START_INTRO_TIME);

    if (lerpAmount < 1.0f) {
        int y = 260 + (int)(lerpAmount * lerpAmount * 400);
        spriteSetColor(renderState, NIGHTCHILDE_INDEX, gColorWhite);
        fontRenderText(renderState, &gNightChilde, "press start", 198, y + 2, 1, 0, NULL, NULL);
        spriteSetColor(renderState, NIGHTCHILDE_INDEX, gColorBlack);
        fontRenderText(renderState, &gNightChilde, "press start", 200, y, 1, 0, NULL, NULL);
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

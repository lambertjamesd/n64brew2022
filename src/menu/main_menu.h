#ifndef __MENU_MAIN_MENU_H__
#define __MENU_MAIN_MENU_H__

#include "../graphics/graphics.h"

enum MainMenuState {
    MainMenuTitleScreen,
    MainMenuLevelList,
    MainMenuCredits,
    MainMenuLoading,
};

struct MainMenu {
    enum MainMenuState currentState;

    float fadeAmount;
    float titleAnimation;
    float windowOpenAnimation;

    int levelToLoad;
    int selectedLevel;

    ALSndId musicSound;
};

extern struct MainMenu gMainMenu;

void mainMenuInit(struct MainMenu* mainMenu);
void mainMenuEnter(struct MainMenu* mainMenu);
void mainMenuShowCredits(struct MainMenu* mainMenu);
void mainMenuUpdate(struct MainMenu* mainMenu);
void mainMenuRender(struct MainMenu* mainMenu, struct RenderState* renderState, struct GraphicsTask* graphicsTask);

#endif
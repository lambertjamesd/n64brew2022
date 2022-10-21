#ifndef __MENU_MAIN_MENU_H__
#define __MENU_MAIN_MENU_H__

#include "../graphics/graphics.h"

struct MainMenu {
    float fadeAmount;
};

extern struct MainMenu gMainMenu;

void mainMenuInit(struct MainMenu* mainMenu);
void mainMenuUpdate(struct MainMenu* mainMenu);
void mainMenuRender(struct MainMenu* mainMenu, struct RenderState* renderState, struct GraphicsTask* graphicsTask);

#endif
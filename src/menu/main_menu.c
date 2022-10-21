#include "main_menu.h"

#include "../graphics/image.h"

#include "../build/assets/materials/ui.h"

struct MainMenu gMainMenu;

void mainMenuInit(struct MainMenu* mainMenu) {
    mainMenu->fadeAmount = 0.0f;
}

void mainMenuUpdate(struct MainMenu* mainMenu) {

}

void mainMenuRender(struct MainMenu* mainMenu, struct RenderState* renderState, struct GraphicsTask* graphicsTask) {
    graphicsCopyImage(renderState, ui_title_screen_rgba_16b, SCREEN_WD, SCREEN_HT, 0, 0, 0, 0, SCREEN_WD, SCREEN_HT, gColorWhite);
}

#ifndef __PAUSE_MENU_H__
#define __PAUSE_MENU_H__

#include "../graphics/renderstate.h"

struct PauseMenu {
    int isEnabled;
    int selectedItem;
    float showAnimation;
};

void pauseMenuInit(struct PauseMenu* pauseMenu);
int pauseMenuUpdate(struct PauseMenu* pauseMenu);
void pauseMenuRender(struct PauseMenu* pauseMenu, struct RenderState* renderState);
int pauseMenuIsPaused(struct PauseMenu* pauseMenu);

#endif
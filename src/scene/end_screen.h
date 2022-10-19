#ifndef __SCENE_END_SCEEN_H__
#define __SCENE_END_SCEEN_H__

#include "../graphics/renderstate.h"

struct EndScreen {
    float preDelay;
    float animationLerp;
    float textAnimation;
    int success;
};

void endScreenInit(struct EndScreen* endScreen);
void endScreenRender(struct EndScreen* endScreen, struct RenderState* renderState);
void endScreenUpdate(struct EndScreen* endScreen);
void endScreenEndame(struct EndScreen* endScreen, int success);
int endSceenIsDone(struct EndScreen* endScreen);
float endScreenFadeAmount(struct EndScreen* endScreen);

#endif
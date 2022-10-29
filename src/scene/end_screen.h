#ifndef __SCENE_END_SCEEN_H__
#define __SCENE_END_SCEEN_H__

#include "../graphics/renderstate.h"

enum EndScreenType {
    EndScreenTypeNone = -1,
    EndScreenTypeSuccess,
    EndScreenTypeFail,
    EndScreenTypeSuccessLastLevel,
};

struct EndScreen {
    float preDelay;
    float animationLerp;
    float textAnimation;
    int success;
};

void endScreenInit(struct EndScreen* endScreen);
void endScreenRender(struct EndScreen* endScreen, struct RenderState* renderState);
int endScreenUpdate(struct EndScreen* endScreen);
void endScreenEndGame(struct EndScreen* endScreen, enum EndScreenType type);
int endScreenIsDone(struct EndScreen* endScreen);
float endScreenFadeAmount(struct EndScreen* endScreen);

#endif
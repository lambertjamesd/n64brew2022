#ifndef __SCENE_TUTORIAL_H__
#define __SCENE_TUTORIAL_H__

#include "../graphics/renderstate.h"

enum TutorialState {
    TutorialStateWait,
    TutorialStatePickup,
    TutorialStateDrop,
    TutorialStateTable,
    TutorialStatePlay,
};

struct Tutorial {
    enum TutorialState state;
    enum TutorialState nextState;

    float animationLerp;
};

void tutorialInit(struct Tutorial* tutorial);

void tutorialSetNextState(struct Tutorial* tutorial, enum TutorialState state);

void tutorialItemPickedUp(struct Tutorial* tutorial);
void tutorialItemDropped(struct Tutorial* tutorial);
void tutorialItemTabled(struct Tutorial* tutorial);

void tutorialUpdate(struct Tutorial* tutorial);

void tutorialRender(struct Tutorial* tutorial, struct RenderState* renderState);

#endif
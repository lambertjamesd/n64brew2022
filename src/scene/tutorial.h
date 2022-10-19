#ifndef __SCENE_TUTORIAL_H__
#define __SCENE_TUTORIAL_H__

#include "../graphics/renderstate.h"

enum TutorialState {
    TutorialStateWait,
    TutorialStateIntro,
    TutorialStatePickup,
    TutorialStatePortalDialog,
    TutorialStateDrop,
    TutorialStateWrongDrop,
    TutorialStateTable,
    TutorialStateSecondDrop,
    TutorialStatePlay,
    TutorialStateCount,
};

struct TutorialScriptStep {
    char* message;
};

struct TutorialScript {
    struct TutorialScriptStep* steps;
    short count;
    short nextState;
};

struct Tutorial {
    enum TutorialState state;
    enum TutorialState nextState;

    struct TutorialScript* currentScript;
    short currentStep;
    short currentStepCharacterCount;
    float currentCharacter;

    float animationLerp;
};

void tutorialInit(struct Tutorial* tutorial);

void tutorialSetNextState(struct Tutorial* tutorial, enum TutorialState state);

void tutorialItemPickedUp(struct Tutorial* tutorial);
void tutorialItemDropped(struct Tutorial* tutorial, int success);
void tutorialItemTabled(struct Tutorial* tutorial);

int tutorialUpdate(struct Tutorial* tutorial);

void tutorialRender(struct Tutorial* tutorial, struct RenderState* renderState);

#endif
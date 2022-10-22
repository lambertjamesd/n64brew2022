#ifndef __SCENE_TUTORIAL_H__
#define __SCENE_TUTORIAL_H__

#include "../graphics/renderstate.h"
#include "../level/level_definition.h"

struct Tutorial {
    struct TutorialStep* tutorial;

    struct TutorialStep* currentStep;
    short currentDialogIndex;

    short currentDialogCharacterCount;
    float currentCharacter;

    enum TutorialPromptType currentPrompt;

    float animationLerp;
    float promptBoxLerp;
};

enum TutorialDropType {
    TutorialDropTypeSuccess,
    TutorialDropTypeFail,
    TutorialDropTypeTable,
};

void tutorialInit(struct Tutorial* tutorial, struct TutorialStep* script, short tutorialOnStart);

void tutorialItemPickedUp(struct Tutorial* tutorial);
void tutorialItemDropped(struct Tutorial* tutorial, enum TutorialDropType dropType);

int tutorialUpdate(struct Tutorial* tutorial);

int tutorialIsImmute(struct Tutorial* tutorial);

void tutorialRender(struct Tutorial* tutorial, struct RenderState* renderState);

#endif
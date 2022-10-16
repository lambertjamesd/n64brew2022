#include "tutorial.h"

#include "../ui/nightchilde.h"

void tutorialInit(struct Tutorial* tutorial) {
    tutorial->state = TutorialStateWait;
    tutorial->nextState = TutorialStateWait;
    tutorial->animationLerp = 0.0f;
}

void tutorialSetNextState(struct Tutorial* tutorial, enum TutorialState state) {

}

void tutorialItemPickedUp(struct Tutorial* tutorial) {

}

void tutorialItemDropped(struct Tutorial* tutorial) {

}

void tutorialItemTabled(struct Tutorial* tutorial) {

}

void tutorialUpdate(struct Tutorial* tutorial) {

}

void tutorialRender(struct Tutorial* tutorial, struct RenderState* renderState) {
    // if (tutorial->state == TutorialStateWait) {
    //     return;
    // }

    fontRenderText(renderState, &gNightChilde, "abcdefghijklm\nnopqrstuvwxyz 012345679", 10, 10, 0);
}
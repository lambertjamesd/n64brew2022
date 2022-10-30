#include "item_coordinator.h"

#include "../math/mathf.h"
#include "../util/time.h"

#include "scene.h"

void itemDeckInit(struct ItemDeck* deck) {
    deck->deckSize = 0;
    deck->readIndex = 0;
}

void itemDeckAddItem(struct ItemDeck* deck, enum ItemType itemType) {
    if (deck->deckSize == MAX_ITEM_DECK_SIZE) {
        return;
    }

    deck->deckEntries[deck->deckSize] = itemType;
    ++deck->deckSize;
}

void itemDeckShuffle(struct ItemDeck* deck) {
    for (int i = 0; i + 1 < deck->deckSize; ++i) {
        int source = randomInRange(i + 1, deck->deckSize);

        int tmp = deck->deckEntries[i];
        deck->deckEntries[i] = deck->deckEntries[source];
        deck->deckEntries[source] = tmp;
    }

    deck->readIndex = 0;
}

enum ItemType itemDeckNextItem(struct ItemDeck* deck) {
    if (deck->deckSize == 0) {
        return ItemTypePumpkin;
    }

    if (deck->readIndex >= deck->deckSize) {
        itemDeckShuffle(deck);
    }

    enum ItemType result = deck->deckEntries[deck->readIndex];
    ++deck->readIndex;
    return result;
}

int itemDeckNeedsShuffle(struct ItemDeck* deck) {
    return deck->readIndex >= deck->deckSize;
}

extern struct Scene gScene;

void itemCoordinatorInitCurrentStep(struct ItemCoordinator* itemCoordinator) {
    if (itemCoordinator->currentScriptStep >= itemCoordinator->script->stepCount) {
        return;
    }
    
    itemDeckInit(&itemCoordinator->itemSource);
    itemDeckInit(&itemCoordinator->itemDrop);

    struct ItemScriptStep* currentStep = &itemCoordinator->script->steps[itemCoordinator->currentScriptStep];

    for (int i = 0; i < currentStep->itemPoolSize; ++i) {
        itemDeckAddItem(&itemCoordinator->itemSource, currentStep->itemPool[i]);
        itemDeckAddItem(&itemCoordinator->itemDrop, currentStep->itemPool[i]);
    }

    itemDeckShuffle(&itemCoordinator->itemSource);
    itemDeckShuffle(&itemCoordinator->itemDrop);

    itemCoordinator->currentSuccessCount = 0;

    if (currentStep->onStart != TUTORAL_NO_STEP) {
        tutorialSetNextStep(&gScene.tutorial, currentStep->onStart);
    }
}

void itemCoordinatorInit(struct ItemCoordinator* itemCoordinator, struct ItemScript* script) {
    itemCoordinator->currentScriptStep = 0;
    itemCoordinator->currentSuccessCount = 0;
    itemCoordinator->script = script;
    itemCoordinator->totalItems = 0;
    itemCoordinator->itemsFulfulled = 0;

    for (int i = 0; i < script->stepCount; ++i) {
        itemCoordinator->totalItems += script->steps[i].successCount;
    }

    itemCoordinatorInitCurrentStep(itemCoordinator);
}

enum ItemType itemCoordinatorNextRequest(struct ItemCoordinator* itemCoordinator, int activeRequesterCount) {
    struct ItemScriptStep* currentStep = &itemCoordinator->script->steps[itemCoordinator->currentScriptStep];

    int requestsLeft = currentStep->successCount - itemCoordinator->currentSuccessCount;

    if (requestsLeft <= activeRequesterCount) {
        return ItemTypeCount;
    }

    return itemDeckNextItem(&itemCoordinator->itemDrop);
}

enum ItemType itemCoordinatorNextArrival(struct ItemCoordinator* itemCoordinator) {
    return itemDeckNextItem(&itemCoordinator->itemSource);
}

int itemCoordinatorDidWin(struct ItemCoordinator* itemCoordinator) {
    return itemCoordinator->currentScriptStep >= itemCoordinator->script->stepCount;
}

void itemCoordinatorMarkSuccess(struct ItemCoordinator* itemCoordinator) {
    if (itemCoordinatorDidWin(itemCoordinator)) {
        return;
    }
    
    ++itemCoordinator->currentSuccessCount;
    ++itemCoordinator->itemsFulfulled;

    struct ItemScriptStep* currentStep = &itemCoordinator->script->steps[itemCoordinator->currentScriptStep];
    
    if (itemCoordinator->currentSuccessCount >= currentStep->successCount) {
        ++itemCoordinator->currentScriptStep;

        if (itemCoordinatorDidWin(itemCoordinator)) {
            return;
        }

        itemCoordinatorInitCurrentStep(itemCoordinator);
    }
}

void itemCoordinatorUpdate(struct ItemCoordinator* itemCoordinator) {

}

float itemCoordinatorTimeout(struct ItemCoordinator* itemCoordinator) {
    if (itemCoordinator->currentScriptStep >= itemCoordinator->script->stepCount) {
        return 30.0f;
    }

    struct ItemScriptStep* currentStep = &itemCoordinator->script->steps[itemCoordinator->currentScriptStep];

    return currentStep->itemTimeout;
}

float itemCoordinatorPreDelay(struct ItemCoordinator* itemCoordinator) {
    if (itemCoordinator->currentScriptStep >= itemCoordinator->script->stepCount) {
        return 3.0f;
    }

    struct ItemScriptStep* currentStep = &itemCoordinator->script->steps[itemCoordinator->currentScriptStep];

    return currentStep->itemDelay;
}

float itemCoordinatorSpawnDelay(struct ItemCoordinator* itemCoordinator) {
    if (itemCoordinator->currentScriptStep >= itemCoordinator->script->stepCount) {
        return 0.0f;
    }

    struct ItemScriptStep* currentStep = &itemCoordinator->script->steps[itemCoordinator->currentScriptStep];

    return currentStep->itemSpawnDelay;
}

short itemCoordinatorMusic(struct ItemCoordinator* itemCoordinator) {
    if (itemCoordinator->currentScriptStep >= itemCoordinator->script->stepCount) {
        return -1;
    }

    struct ItemScriptStep* currentStep = &itemCoordinator->script->steps[itemCoordinator->currentScriptStep];

    return currentStep->music;
}
#ifndef __SCENE_ITEM_COORDINATOR_H__
#define __SCENE_ITEM_COORDINATOR_H__

#include "item.h"
#include "../level/level_definition.h"

#define MAX_ITEM_DECK_SIZE  64

struct ItemDeck {
    u8 deckEntries[MAX_ITEM_DECK_SIZE];
    u8 deckSize;
    u8 readIndex;
};

void itemDeckInit(struct ItemDeck* deck);
void itemDeckAddItem(struct ItemDeck* deck, enum ItemType itemType);
void itemDeckShuffle(struct ItemDeck* deck);

enum ItemType itemDeckNextItem(struct ItemDeck* deck);

int itemDeckNeedsShuffle(struct ItemDeck* deck);

struct ItemCoordinator {
    struct ItemDeck itemSource;
    struct ItemDeck itemDrop;
    struct ItemScript* script;
    short currentScriptStep;
    short currentSuccessCount;
    short activeRequesterCount;
};

void itemCoordinatorInit(struct ItemCoordinator* itemCoordinator, struct ItemScript* script);

enum ItemType itemCoordinatorNextRequest(struct ItemCoordinator* itemCoordinator, int activeRequesterCount);
enum ItemType itemCoordinatorNextArrival(struct ItemCoordinator* itemCoordinator);

int itemCoordinatorDidWin(struct ItemCoordinator* itemCoordinator);

void itemCoordinatorMarkSuccess(struct ItemCoordinator* itemCoordinator);

void itemCoordinatorUpdate(struct ItemCoordinator* itemCoordinator);

#endif
#ifndef __SCENE__H
#define __SCENE__H

#include "../graphics/renderstate.h"
#include "../graphics/graphics.h"

#include "../level/level_definition.h"

#include "point_light.h"
#include "camera.h"
#include "player.h"
#include "spot_light.h"
#include "conveyor.h"
#include "item.h"
#include "table.h"
#include "item_requester.h"
#include "bezos.h"
#include "item_coordinator.h"
#include "tutorial.h"
#include "end_screen.h"
#include "return_bin.h"

#include "../collision/collision_boundary.h"

typedef void (*SetObjectMaterial)(struct RenderState* renderState, int objectIndex);

enum RenderModeFlags {
    RenderModeFlagsAttenuate = (1 << 0),
};

struct RenderModeData {
    enum RenderModeFlags flags;
    u64* pallete;
    u32 clearColor;
    SetObjectMaterial setObjectMaterial;
    SetObjectMaterial secondObjectPass;
    SetObjectMaterial groundMaterial;
};

struct Scene {
    struct Camera camera;
    struct Player players[MAX_PLAYERS];
    struct ItemSlot* itemSlots;
    struct SpotLight* spotLights;
    struct Conveyor* conveyors;
    struct Table* tables;
    struct ItemRequester* itemRequesters;
    struct ItemPool itemPool;
    struct Bezos bezos;
    struct ItemCoordinator itemCoordinator;
    struct Tutorial tutorial;
    struct EndScreen endScreen;
    struct ReturnBin* returnBins;
    u8 itemSlotCount;
    u8 playerCount;
    u8 spotLightCount;
    u8 conveyorCount;
    u8 tableCount;
    u8 itemRequesterCount;
    u8 returnBinCount;

    float dropPenalty;
    float appearTime;
    float fadeInTime;
    float currentLevelTime;
    float penaltyTime;
};

void sceneInit(struct Scene* scene, struct LevelDefinition* definition, int playerCount);
void sceneUpdate(struct Scene* scene);
void sceneRender(struct Scene* scene, struct RenderState* renderState, struct GraphicsTask* task);

struct Item* scenePickupItem(struct Scene* scene, struct Vector3* grabFrom);

int sceneDropItem(struct Scene* scene, struct Item* item, struct Vector3* dropAt);
int sceneSwapItem(struct Scene* scene, struct Item* item, struct Vector3* dropAt, struct Item** replacement);
int sceneItemHover(struct Scene* scene, struct Item* item, struct Vector3* dropAt, struct Vector3* hoverOutput);

struct Vector3* sceneNearestPlayerPos(struct Scene* scene);

#endif
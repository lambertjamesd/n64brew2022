#ifndef __LEVEL_DEFINITION_H__
#define __LEVEL_DEFINITION_H__

#include <ultra64.h>

#include "../math/transform.h"

#define MAX_PLAYERS     4

enum ItemType {
    ItemTypePumpkin,
    ItemTypeHat,
    ItemTypeBrain,
    ItemTypeBroom,
    ItemTypeCandle,
    ItemTypeCat,
    ItemTypeCobweb,
    ItemTypeCrow,
    ItemTypeHand,
    ItemTypeRat,
    ItemTypeScarecrow,
    ItemTypeSkull,
    ItemTypeSpider,

    ItemTypeCount,
};

struct StaticContentElement {
    Gfx* displayList;
    u8 materialIndex;
};

struct PlayerStartLocation {
    struct Vector3 position;
};

struct ItemSlotDefinition {
    struct Vector3 position;
};

struct CameraDefinition {
    struct Vector3 position;
    struct Quaternion rotation;
    float verticalFov;
    float nearPlane;
    float farPlane;
};

struct SpotLightDefinition {
    struct Vector3 position;
    struct Quaternion rotation;
    float angle;
};

struct ConveyorDefinition {
    struct Vector3 position;
    struct Quaternion rotation;
};

struct TableDefinition {
    struct Vector3 position;
    short tableType;
};

struct ItemRequesterDefinition {
    struct Vector3 position;
    struct Quaternion rotation;
};

struct ItemScriptStep {
    u8 itemPoolSize;
    u8 successCount;
    u8* itemPool;
    float itemTimeout;
    float itemDelay;
};

struct ItemScript {
      struct ItemScriptStep* steps;
      short stepCount;
};

struct LevelDefinition {
    struct StaticContentElement* staticContent;
    short staticContentCount;

    struct StaticContentElement* groundContent;
    short groundContentCount;

    struct ItemSlotDefinition* itemSlots;
    short itemSlotCount;

    struct SpotLightDefinition* spotLights;
    short spotLightCount;

    struct ConveyorDefinition* conveyors;
    short conveyorCount;

    struct TableDefinition* tables;
    short tableCount;

    struct ItemRequesterDefinition* itemRequesters;
    short itemRequesterCount;

    struct PlayerStartLocation playerStart[MAX_PLAYERS];
    struct CameraDefinition cameraDefinition;

    struct ItemScript* script;
};

struct LevelDefinition* levelFixPointers(struct LevelDefinition* from, int pointerOffset);

#endif
#ifndef __LEVEL_DEFINITION_H__
#define __LEVEL_DEFINITION_H__

#include <ultra64.h>

#include "../math/transform.h"

#define MAX_PLAYERS     4

#define TUTORAL_NO_STEP -1

enum TutorialPromptType {
    TutorialPromptTypeNone,
    TutorialPromptTypePickup,
    TutorialPromptTypeDrop,
};

struct TutorialDialogStep {
    char* message;
};

struct TutorialStep {
    struct TutorialDialogStep* dialog;
    short dialogCount;
    short nextState;
    short onSuccess;
    short onFail;
    short onTable;
    short onPickup;
    short prompt;
    short isImmune;
};

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

#define TABLE_HORIZONTAL    0
#define TABLE_VERTICAL      1

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
    float itemSpawnDelay;
};

struct ItemScript {
      struct ItemScriptStep* steps;
      short stepCount;
};

struct BoundarySegment {
    struct Vector2 a;
    struct Vector2 b;
};

struct ReturnBinDefinition {
    struct Vector3 position;
};

struct LevelDefinition {
    struct StaticContentElement* staticContent;
    short staticContentCount;

    struct StaticContentElement* groundContent;
    short groundContentCount;

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

    struct BoundarySegment* boundary;
    short boundaryCount;

    struct TutorialStep* tutorial;
    short tutorialStepCount;
    short tutorialOnStart;

    struct ReturnBinDefinition* returnBins;
    short returnBinCount;
};

struct LevelDefinition* levelFixPointers(struct LevelDefinition* from, int pointerOffset);

#endif
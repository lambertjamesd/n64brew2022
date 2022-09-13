#ifndef __LEVEL_DEFINITION_H__
#define __LEVEL_DEFINITION_H__

#include <ultra64.h>

#include "../math/transform.h"

#define MAX_PLAYERS     4

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
};

struct LevelDefinition {
    struct StaticContentElement* staticContent;
    short staticContentCount;

    struct ItemSlotDefinition* itemSlots;
    short itemSlotCount;

    struct PlayerStartLocation playerStart[MAX_PLAYERS];
    struct CameraDefinition cameraDefinition;
};

struct LevelDefinition* levelFixPointers(struct LevelDefinition* from, int pointerOffset);

#endif
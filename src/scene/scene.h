#ifndef __SCENE__H
#define __SCENE__H

#include "../graphics/renderstate.h"
#include "../graphics/graphics.h"

#include "../level/level_definition.h"

#include "point_light.h"
#include "camera.h"
#include "player.h"
#include "item_slot.h"
#include "spot_light.h"

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
    u8 itemSlotCount;
    u8 playerCount;
    u8 spotLightCount;
};

void sceneInit(struct Scene* scene, struct LevelDefinition* definition, int playerCount);
void sceneUpdate(struct Scene* scene);
void sceneRender(struct Scene* scene, struct RenderState* renderState, struct GraphicsTask* task);

#endif
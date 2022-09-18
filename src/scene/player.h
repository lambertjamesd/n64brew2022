#ifndef _SCENE_PLAYER_H_
#define _SCENE_PLAYER_H_

#include "../math/transform.h"
#include "../math/vector2.h"

#include "../level/level_definition.h"

#include "../graphics/render_scene.h"

#include "../sk64/skelatool_armature.h"

#include "shadow_map.h"

struct Player {
    struct Transform transform;
    struct SKArmature armature;
    short playerIndex;

    struct Vector3 velocity;
    struct Vector2 lookDir;

    struct ShadowMap shadowMap;

    Mtx* mtxTransform;
    Mtx* mtxArmature;
};

void playerInit(struct Player* player, struct PlayerStartLocation* startLocation, int index, u16* buffer);

void playerUpdate(struct Player* player);
void playerSetupTransforms(struct Player* player, struct RenderState* renderState);
void playerRender(struct Player* player, struct RenderScene* renderScene);

Gfx* playerGenerateShadowMapGfx(struct Player* player, struct RenderState* renderState);

#endif
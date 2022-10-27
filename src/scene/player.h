#ifndef _SCENE_PLAYER_H_
#define _SCENE_PLAYER_H_

#include "../math/transform.h"
#include "../math/vector2.h"

#include "../level/level_definition.h"

#include "../graphics/render_scene.h"

#include "../sk64/skelatool_armature.h"
#include "../sk64/skelatool_animator.h"

#include "../collision/collision_object.h"

#include "./shadow_volume_group.h"

#include "shadow_map.h"
#include "item.h"

struct Player {
    struct Transform transform;
    struct SKArmature armature;
    struct SKAnimator animator;

    short playerIndex;
    short isDead;

    struct Vector3 velocity;
    struct Vector2 lookDir;

    struct ShadowMap shadowMap;

    Mtx* mtxTransform;
    Mtx* mtxArmature;

    float animationSpeed;

    struct Item* holdingItem;
    struct Item* usingItem;

    struct CollisionCapsule collider;

    struct Vector3 hoverLocation;
};

void playerInit(struct Player* player, struct PlayerStartLocation* startLocation, int index, u16* buffer);

void playerUpdate(struct Player* player);
void playerSetupTransforms(struct Player* player, struct RenderState* renderState);
void playerRender(struct Player* player, Light* light, struct RenderScene* renderScene);

int playerCanGrab(struct Player* player);

void playerGrabPoint(struct Player* player, struct Vector3* grabFrom);

void playerHandObject(struct Player* player, struct Item* holdingItem);

Gfx* playerGenerateShadowMapGfx(struct Player* player, struct RenderState* renderState);

void playerToShadowTarget(struct Player* player, struct ShadowVolumeTarget* target, Light* light);

void playerKill(struct Player* player);

int playerIsUsingItem(struct Player* player);
void playerStopUsingItem(struct Player* player);

#endif
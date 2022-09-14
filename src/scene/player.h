#ifndef _SCENE_PLAYER_H_
#define _SCENE_PLAYER_H_

#include "../math/transform.h"

#include "../level/level_definition.h"

#include "../graphics/render_scene.h"

#include "../sk64/skelatool_armature.h"

struct Player {
    struct Transform transform;
    struct SKArmature armature;
    short playerIndex;

    struct Vector3 velocity;
};

void playerInit(struct Player* player, struct PlayerStartLocation* startLocation, int index);

void playerUpdate(struct Player* player);

void playerRender(struct Player* player, struct RenderScene* renderScene);

#endif
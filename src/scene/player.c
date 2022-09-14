#include "player.h"

#include "../defs.h"

#include "../controls/controller.h"
#include "../util/time.h"

#include "../build/assets/models/player.h"
#include "../build/assets/materials/static.h"

#define PLAYER_MOVE_SPEED   2.0f
#define PLAYER_ACCELERATION 10.0f

void playerInit(struct Player* player, struct PlayerStartLocation* startLocation, int index) {
    player->transform.position = startLocation->position;
    quatIdent(&player->transform.rotation);
    player->transform.scale = gOneVec;
    player->playerIndex = index;

    skArmatureInit(
        &player->armature, 
        player_model_gfx, 
        PLAYER_DEFAULT_BONES_COUNT, 
        player_default_bones, 
        player_bone_parent, 
        PLAYER_ATTACHMENT_COUNT
    );
}

void playerUpdate(struct Player* player) {
    OSContPad* input = controllersGetControllerData(player->playerIndex);

    struct Vector3 moveDir;
    moveDir.x = input->stick_x * (1.0f / 80.0f);
    moveDir.y = 0.0f;
    moveDir.z = -input->stick_y * (1.0f / 80.0f);

    float magSqrd = vector3MagSqrd(&moveDir);

    if (magSqrd > 1.0f) {
        vector3Scale(&moveDir, &moveDir, PLAYER_MOVE_SPEED / sqrtf(magSqrd));
    } else {
        vector3Scale(&moveDir, &moveDir, PLAYER_MOVE_SPEED);
    }

    vector3MoveTowards(&player->velocity, &moveDir, PLAYER_ACCELERATION * FIXED_DELTA_TIME, &player->velocity);

    vector3AddScaled(&player->transform.position, &player->velocity, FIXED_DELTA_TIME, &player->transform.position);
}

void playerRender(struct Player* player, struct RenderScene* renderScene) {
    Mtx* transform = renderStateRequestMatrices(renderScene->renderState, 1);
    Mtx* armature = renderStateRequestMatrices(renderScene->renderState, PLAYER_DEFAULT_BONES_COUNT);

    transformToMatrixL(&player->transform, transform, SCENE_SCALE);

    skCalculateTransforms(&player->armature, armature);

    Gfx* attachments = skBuildAttachments(&player->armature, NULL, renderScene->renderState);

    Gfx* objectRender = renderStateAllocateDLChunk(renderScene->renderState, 3);
    Gfx* dl = objectRender;

    if (attachments) {
        gSPSegment(dl++, BONE_ATTACHMENT_SEGMENT,  osVirtualToPhysical(attachments));
    }
    gSPDisplayList(dl++, player->armature.displayList);
    gSPEndDisplayList(dl++);

    renderSceneAdd(
        renderScene, 
        objectRender, 
        transform, 
        PLAYER_0_INDEX, 
        &player->transform.position, 
        armature
    );
}
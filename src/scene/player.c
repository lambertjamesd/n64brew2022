#include "player.h"

#include "../defs.h"

#include "../build/assets/models/player.h"
#include "../build/assets/materials/static.h"

void playerInit(struct Player* player, struct PlayerStartLocation* startLocation) {
    player->transform.position = startLocation->position;
    player->transform.rotation = startLocation->rotation;
    player->transform.scale = gOneVec;

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
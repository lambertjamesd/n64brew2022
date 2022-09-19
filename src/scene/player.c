#include "player.h"

#include "../defs.h"

#include "../controls/controller.h"
#include "../util/time.h"

#include "../build/assets/models/player.h"
#include "../build/assets/materials/static.h"

#define PLAYER_MOVE_SPEED   2.0f

#define PLAYER_MAX_SPRINT_SPEED 4.0f
#define PLAYER_MAX_WALK_SPEED   1.0f

#define PLAYER_ACCELERATION 10.0f

#define PLAYER_ROTATE_RATE  (M_PI * 2.0f)

struct Vector2 gMaxRotateVector;

struct Vector3 gPlayerCenter = {0.0f, 0.8f, 0.0f};

struct SKAnimationHeader* playerDetermineAnimation(struct Player* player, float* playbackSpeed) {
    float speedSqrd = sqrtf(vector3MagSqrd(&player->velocity));

    if (speedSqrd < 0.00001f) {
        *playbackSpeed = 0.0f;
        return NULL;
    } else if (speedSqrd < PLAYER_MAX_WALK_SPEED) {
        *playbackSpeed = speedSqrd * (1.0f / PLAYER_MAX_WALK_SPEED);
        return &player_animations[PLAYER_PLAYER__PLAYER_0_WALK_INDEX];
    } else {
        *playbackSpeed = speedSqrd * (1.0f / PLAYER_MAX_SPRINT_SPEED);
        return &player_animations[PLAYER_PLAYER__PLAYER_0_SPRINT_INDEX];
    }
}

void playerInit(struct Player* player, struct PlayerStartLocation* startLocation, int index, u16* buffer) {
    player->transform.position = startLocation->position;
    quatIdent(&player->transform.rotation);
    player->transform.scale = gOneVec;
    player->playerIndex = index;
    player->animationSpeed = 0.0f;

    skArmatureInit(
        &player->armature, 
        player_model_gfx, 
        PLAYER_DEFAULT_BONES_COUNT, 
        player_default_bones, 
        player_bone_parent, 
        PLAYER_ATTACHMENT_COUNT
    );

    skAnimatorInit(
        &player->animator,
        PLAYER_DEFAULT_BONES_COUNT,
        NULL,
        NULL
    );

    gMaxRotateVector.x = cosf(PLAYER_ROTATE_RATE * FIXED_DELTA_TIME);
    gMaxRotateVector.y = sinf(PLAYER_ROTATE_RATE * FIXED_DELTA_TIME);

    player->lookDir.x = 1.0f;
    player->lookDir.y = 0.0f;

    shadowMapInit(&player->shadowMap, &gPlayerCenter, 0.5f, 0.5f, 10.0f, buffer);
}

void playerHandleRotation(struct Player* player, struct Vector3* moveDir) {
    struct Vector2 rotateTowards;
    rotateTowards.x = moveDir->z;
    rotateTowards.y = moveDir->x;

    if (!vector2Normalize(&rotateTowards, &rotateTowards)) {
        return;
    }

    vector2RotateTowards(&player->lookDir, &rotateTowards, &gMaxRotateVector, &player->lookDir);

    quatAxisComplex(&gUp, &player->lookDir, &player->transform.rotation);
}

void playerUpdate(struct Player* player) {
    skAnimatorUpdate(&player->animator, player->armature.boneTransforms, player->animationSpeed);

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

    if (magSqrd > 0.0f) {
        playerHandleRotation(player, &moveDir);
    }

    struct SKAnimationHeader* nextAnimation = playerDetermineAnimation(player, &player->animationSpeed);

    if (nextAnimation != player->animator.currentAnimation && nextAnimation) {
        skAnimatorRunClip(&player->animator, nextAnimation, SKAnimatorFlagsLoop);
    }
}

void playerSetupTransforms(struct Player* player, struct RenderState* renderState) {
    Mtx* transform = renderStateRequestMatrices(renderState, 1);
    Mtx* armature = renderStateRequestMatrices(renderState, PLAYER_DEFAULT_BONES_COUNT);

    transformToMatrixL(&player->transform, transform, SCENE_SCALE);

    skCalculateTransforms(&player->armature, armature);

    player->mtxTransform = transform;
    player->mtxArmature = armature;
}

void playerRender(struct Player* player, struct RenderScene* renderScene) {
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
        player->mtxTransform, 
        PLAYER_0_INDEX, 
        &player->transform.position, 
        player->mtxArmature
    );
}

Gfx* playerGenerateShadowMapGfx(struct Player* player, struct RenderState* renderState) {
    Gfx* result = renderStateAllocateDLChunk(renderState, 4);
    Gfx* dl = result;

    gSPSegment(dl++, MATRIX_TRANSFORM_SEGMENT,  osVirtualToPhysical(player->mtxArmature));
    gSPSegment(dl++, BONE_ATTACHMENT_SEGMENT,  skBuildAttachments(&player->armature, NULL, renderState));
    gSPDisplayList(dl++, player->armature.displayList);
    gSPEndDisplayList(dl++);

    return result;
}
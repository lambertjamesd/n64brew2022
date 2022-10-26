#include "player.h"

#include "../defs.h"

#include "../controls/controller.h"
#include "../util/time.h"

#include "../build/assets/models/player.h"
#include "../build/assets/materials/static.h"
#include "../build/assets/models/ui/item_drop_prompt.h"

#include "../collision/collision_scene.h"

#define PLAYER_BROOM_SPEED  2.8f

#define PLAYER_MOVE_SPEED   1.4f

#define PLAYER_MAX_SPRINT_SPEED 4.0f
#define PLAYER_MAX_WALK_SPEED   1.0f

#define PLAYER_ACCELERATION 8.0f

#define PLAYER_ROTATE_RATE  (M_PI * 1.5f)

#define COLLIDER_RADIUS     0.25f
#define COLLIDER_HEIGHT     0.25f

#define PLAYER_GRAVITY      -9.8f

struct Vector2 gMaxRotateVector;

struct Vector3 gPlayerCenter = {0.0f, 0.8f, 0.0f};

struct Vector3 gPlayerGrabFrom = {0.0f, 0.0f, 0.4f};

struct Vector3 gAttachmentPosition = {0.0f, 0.0f, 0.0f};
struct Quaternion gAttachementRotation = {0.0f, 0.0f, 0.0f, 1.0f};

struct SKAnimationHeader* playerDetermineAnimation(struct Player* player, float* playbackSpeed, int* loop) {
    float speedSqrd = sqrtf(player->velocity.x * player->velocity.x + player->velocity.z * player->velocity.z);
    *loop = 1;

    if (player->isDead) {
        *playbackSpeed = 1.0f;
        *loop = 0;
        return &player_animations[PLAYER_PLAYER__PLAYER_0_PLAYERDEAD_INDEX];
    } else if (speedSqrd < 0.00001f) {
        *playbackSpeed = 1.0f;

        if (player->holdingItem) {
            return &player_animations[PLAYER_PLAYER__PLAYER_0_PLAYERIDLE_W_ITEM_INDEX];
        }

        return &player_animations[PLAYER_PLAYER__PLAYER_0_PLAYERIDLE_INDEX];
    } else if (speedSqrd < PLAYER_MAX_WALK_SPEED) {
        *playbackSpeed = speedSqrd * (1.0f / PLAYER_MAX_WALK_SPEED);

        if (player->holdingItem) {
            return &player_animations[PLAYER_PLAYER__PLAYER_0_WALK_W_ITEM_INDEX];
        }

        return &player_animations[PLAYER_PLAYER__PLAYER_0_WALK_INDEX];
    } else {
        *playbackSpeed = speedSqrd * (1.0f / PLAYER_MAX_SPRINT_SPEED);

        if (player->holdingItem) {
            return &player_animations[PLAYER_PLAYER__PLAYER_0_SPRINT_W_ITEM_INDEX];
        }

        return &player_animations[PLAYER_PLAYER__PLAYER_0_SPRINT_INDEX];
    }
}

void playerUpdateColliderPos(struct Player* player) {
    vector3AddScaled(&player->transform.position, &gUp, COLLIDER_HEIGHT * 0.5f + COLLIDER_RADIUS, &player->collider.center);
    collisionCapsuleUpdateBB(&player->collider);
}

void playerColliderCallback(void* data, struct Vector3* normal, float depth, struct CollisionObject* other) {
    struct Player* player = (struct Player*)data;

    vector3AddScaled(&player->transform.position, normal, depth * 0.75f, &player->transform.position);

    if (vector3Dot(&player->velocity, normal) > 0.0f) {
        vector3ProjectPlane(&player->velocity, normal, &player->velocity);
    }
}

void playerInit(struct Player* player, struct PlayerStartLocation* startLocation, int index, u16* buffer) {
    player->transform.position = startLocation->position;
    quatIdent(&player->transform.rotation);
    player->transform.scale = gOneVec;
    player->playerIndex = index;
    player->isDead = 0;
    player->animationSpeed = 0.0f;
    player->holdingItem = NULL;
    player->velocity = gZeroVec;

    player->hoverLocation = gZeroVec;
    player->hoverLocation.y = -1.0f;

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

    collisionCapsuleInit(&player->collider, COLLIDER_HEIGHT, COLLIDER_RADIUS);
    playerUpdateColliderPos(player);

    player->collider.collisionObject.flags = CollisionObjectFlagsIsPlayer;

    collisionSceneAddDynamic(&gCollisionScene, &player->collider.collisionObject, playerColliderCallback, player);
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

#define DEAD_ZONE   0.2f

void playerUpdate(struct Player* player) {
    skAnimatorUpdate(&player->animator, player->armature.boneTransforms, player->animationSpeed);

    OSContPad* input = controllersGetControllerData(player->playerIndex);

    struct Vector3 moveDir;
    struct Vector3 rotateDir;
    moveDir.x = input->stick_x * (1.0f / 80.0f);
    moveDir.y = 0.0f;
    moveDir.z = -input->stick_y * (1.0f / 80.0f);

    if (player->isDead) {
        moveDir = gZeroVec;
    }

    float magSqrd = vector3MagSqrd(&moveDir);

    float moveSpeed = player->holdingItem && player->holdingItem->type == ItemTypeBroom ? PLAYER_BROOM_SPEED : PLAYER_MOVE_SPEED;

    if (magSqrd > 1.0f) {
        vector3Scale(&moveDir, &moveDir, moveSpeed / sqrtf(magSqrd));
        rotateDir = moveDir;
    } else if (magSqrd < DEAD_ZONE * DEAD_ZONE) {
        rotateDir = moveDir;
        moveDir = gZeroVec;
    } else {
        vector3Scale(&moveDir, &moveDir, moveSpeed);
        rotateDir = moveDir;
    }

    moveDir.y = player->velocity.y;

    vector3MoveTowards(&player->velocity, &moveDir, PLAYER_ACCELERATION * FIXED_DELTA_TIME, &player->velocity);

    player->velocity.y += PLAYER_GRAVITY * FIXED_DELTA_TIME;
    vector3AddScaled(&player->transform.position, &player->velocity, FIXED_DELTA_TIME, &player->transform.position);

    if (magSqrd > 0.0f) {
        playerHandleRotation(player, &rotateDir);
    }

    if (player->transform.position.y < 0.0f) {
        player->transform.position.y = 0.0f;
        player->velocity.y = 0.0f;
    }

    int loop;
    struct SKAnimationHeader* nextAnimation = playerDetermineAnimation(player, &player->animationSpeed, &loop);

    if (nextAnimation != player->animator.currentAnimation && nextAnimation) {
        skAnimatorRunClip(&player->animator, nextAnimation, loop ? SKAnimatorFlagsLoop : 0);
    }

    if (player->holdingItem) {
        struct Transform targetTransform;
        skCalculateBoneTransform(
            &player->armature, 
            PLAYER_ATTACHEMENT_RIGHTHAND_BONE, 
            &targetTransform
        );

        struct Transform combined;

        vector3Scale(&targetTransform.position, &targetTransform.position, 1.0f / SCENE_SCALE);

        transformConcat(&player->transform, &targetTransform, &combined);

        struct Transform* relativeTransform = gItemDefinitions[player->holdingItem->type].grabTransform;

        if (relativeTransform) {
            struct Transform holdingTransform;
            struct Transform relativeInverse;
            transformInvert(relativeTransform, &relativeInverse);
            transformConcat(&combined, &relativeInverse, &holdingTransform);
            combined = holdingTransform;
        }

        itemUpdateTarget(player->holdingItem, &combined);
    }

    playerUpdateColliderPos(player);
}

void playerSetupTransforms(struct Player* player, struct RenderState* renderState) {
    Mtx* transform = renderStateRequestMatrices(renderState, 1);
    Mtx* armature = renderStateRequestMatrices(renderState, PLAYER_DEFAULT_BONES_COUNT);

    transformToMatrixL(&player->transform, transform, SCENE_SCALE);

    skCalculateTransforms(&player->armature, armature);

    player->mtxTransform = transform;
    player->mtxArmature = armature;
}

Light gGemLight = {{{255, 255, 255}, 0, {255, 255, 255}, 0, {88, 88, 0}, 0}};

void playerRender(struct Player* player, Light* light, struct RenderScene* renderScene) {
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
        player->mtxArmature,
        light
    );

    if (player->hoverLocation.y >= 0.0f) {
        struct Transform hoverTransform;
        hoverTransform.position = player->hoverLocation;
        hoverTransform.position.y += (sinf(gTimePassed) + 1.0f) * 0.25f;
        quatAxisAngle(&gUp, gTimePassed * 5.0f, &hoverTransform.rotation);
        hoverTransform.scale = gOneVec;


        Mtx* hoverMtx = renderStateRequestMatrices(renderScene->renderState, 1);
        transformToMatrixL(&hoverTransform, hoverMtx, SCENE_SCALE);

        renderSceneAdd(renderScene, ui_item_drop_prompt_model_gfx, hoverMtx, ITEM_DROP_PROMPT_INDEX, &player->hoverLocation, NULL, &gGemLight);
    }
}


int playerCanGrab(struct Player* player) {
    return !player->holdingItem;
}

void playerGrabPoint(struct Player* player, struct Vector3* grabFrom) {
    return transformPoint(&player->transform, &gPlayerGrabFrom, grabFrom);
}

void playerHandObject(struct Player* player, struct Item* holdingItem) {
    if (player->holdingItem) {
        itemDrop(player->holdingItem);
        player->holdingItem = NULL;
    }

    player->holdingItem = holdingItem;
}

Gfx* playerGenerateShadowMapGfx(struct Player* player, struct RenderState* renderState) {
    Gfx* result = renderStateAllocateDLChunk(renderState, 7);
    Gfx* dl = result;

    gSPSegment(dl++, MATRIX_TRANSFORM_SEGMENT,  osVirtualToPhysical(player->mtxArmature));
    gSPSegment(dl++, BONE_ATTACHMENT_SEGMENT,  skBuildAttachments(&player->armature, NULL, renderState));
    gSPDisplayList(dl++, player->armature.displayList);

    if (player->holdingItem) {
        gSPMatrix(dl++, player->holdingItem->mtxTransform, G_MTX_MODELVIEW | G_MTX_NOPUSH | G_MTX_LOAD);
        gSPSegment(dl++, MATRIX_TRANSFORM_SEGMENT,  player->holdingItem->mtxArmature);
        gSPDisplayList(dl++, gItemDefinitions[player->holdingItem->type].dl);
    }

    gSPEndDisplayList(dl++);

    return result;
}

void playerToShadowTarget(struct Player* player, struct ShadowVolumeTarget* target, Light* light) {
    target->armature = player->mtxArmature;
    target->collisionObject = &player->collider.collisionObject;
    target->displayList = player->armature.displayList;
    target->light = light;
    target->materialIndex = PLAYER_0_INDEX;
    target->matrix = player->mtxTransform;
    target->position = player->transform.position;
    vector3Add(&target->position, &gPlayerCenter, &target->position);
}

void playerKill(struct Player* player) {
    player->isDead = 1;

    if (player->holdingItem) {
        itemDrop(player->holdingItem);
        player->holdingItem = NULL;
    }
}
#include "bezos.h"

#include "../build/assets/models/ghostjeff.h"
#include "../build/assets/models/pumpkin.h"
#include "../build/assets/materials/static.h"

#include "../util/time.h"

#include "../collision/collision_scene.h"

#include "../defs.h"

#define COLLIDER_RADIUS     0.25f
#define COLLIDER_HEIGHT     0.25f

#define BEZOS_MOVE_SPEED   0.5f

#define BEZOS_ACCELERATION 2.0f


void bezosColliderCallback(void* data, struct Vector3* normal, float depth, struct CollisionObject* other) {
    struct Bezos* bezos = (struct Bezos*)data;

    vector3AddScaled(&bezos->transform.position, normal, depth * 0.75f, &bezos->transform.position);

    if (vector3Dot(&bezos->velocity, normal) > 0.0f) {
        vector3ProjectPlane(&bezos->velocity, normal, &bezos->velocity);
    }

    bezos->flags |= BezosFlagsTouchingWall;

    if (other->flags & CollisionObjectFlagsIsPlayer) {
        bezos->flags |= BezosFlagsCaughtPlayer;
    }
}

void bezosUpdateColliderPos(struct Bezos* bezos) {
    vector3AddScaled(&bezos->transform.position, &gUp, COLLIDER_HEIGHT * 0.5f + COLLIDER_RADIUS, &bezos->collider.center);
    collisionCapsuleUpdateBB(&bezos->collider);
}

void bezosInit(struct Bezos* bezos) {
    transformInitIdentity(&bezos->transform);

    skAnimatorInit(&bezos->animator, GHOSTJEFF_DEFAULT_BONES_COUNT, NULL, NULL);
    skArmatureInit(&bezos->armature, ghostjeff_model_gfx, GHOSTJEFF_DEFAULT_BONES_COUNT, ghostjeff_default_bones, ghostjeff_bone_parent, GHOSTJEFF_ATTACHMENT_COUNT);

    bezos->flags = 0;
    bezos->velocity = gZeroVec;

    collisionCapsuleInit(&bezos->collider, COLLIDER_HEIGHT, COLLIDER_RADIUS);
    bezosUpdateColliderPos(bezos);
    bezos->collider.center.y = -100.0f;

    collisionSceneAddDynamic(&gCollisionScene, &bezos->collider.collisionObject, bezosColliderCallback, bezos);
}

void bezosActivate(struct Bezos* bezos, struct Vector3* at) {
    transformInitIdentity(&bezos->transform);
    bezos->transform.position = *at;
    bezos->transform.position.y = 0.0f;

    skAnimatorRunClip(&bezos->animator, &ghostjeff_animations[GHOSTJEFF_GHOSTJEFF_JEFF_ARMATURE_GHOSTATTACKTURN_INDEX], 0);

    bezos->flags |= BezosFlagsWaking;
}

void bezosDeactivate(struct Bezos* bezos) {
    skAnimatorRunClip(&bezos->animator, &ghostjeff_animations[GHOSTJEFF_GHOSTJEFF_JEFF_ARMATURE_GHOSTIDLE_INDEX], 0);

    bezos->flags &= ~BezosFlagsActive;
}

void bezosUpdate(struct Bezos* bezos, struct Vector3* nearestPlayerPos) {
    skAnimatorUpdate(&bezos->animator, bezos->armature.boneTransforms, 1.0f);

    if (bezos->flags & BezosFlagsWaking) {
        if (!skAnimatorIsRunning(&bezos->animator)) {
            bezos->flags &= ~BezosFlagsWaking;
            bezos->flags |= BezosFlagsActive;
            skAnimatorRunClip(&bezos->animator, &ghostjeff_animations[GHOSTJEFF_GHOSTJEFF_JEFF_ARMATURE_GHOSTIDLE_INDEX], SKAnimatorFlagsLoop);
        }

        return;
    }

    if (!(bezos->flags & BezosFlagsActive)) {
        bezos->collider.center.y = -100.0f;
        return;
    }

    if (bezos->flags & BezosFlagsCaughtPlayer) {
        skAnimatorRunClip(&bezos->animator, &ghostjeff_animations[GHOSTJEFF_GHOSTJEFF_JEFF_ARMATURE_GHOSTATTACKTURN_INDEX], SKAnimatorFlagsLoop);
        return;
    }

    struct Vector3 moveDir;
    vector3Sub(nearestPlayerPos, &bezos->transform.position, &moveDir);
    moveDir.y = 0.0f;
    vector3Normalize(&moveDir, &moveDir);

    struct Vector3 backDir;
    vector3Negate(&moveDir, &backDir);
    quatLook(&backDir, &gUp, &bezos->transform.rotation);

    moveDir.y = bezos->velocity.y;

    vector3MoveTowards(&bezos->velocity, &moveDir, BEZOS_MOVE_SPEED * BEZOS_ACCELERATION * FIXED_DELTA_TIME, &bezos->velocity);

    if (bezos->flags & BezosFlagsTouchingWall) {
        bezos->velocity.y = 0.5f;
        bezos->flags &= ~BezosFlagsTouchingWall;
    } else {
        bezos->velocity.y += -9.8f * FIXED_DELTA_TIME;
    }
    vector3AddScaled(&bezos->transform.position, &bezos->velocity, FIXED_DELTA_TIME, &bezos->transform.position);

    if (bezos->transform.position.y < 0.0f) {
        bezos->transform.position.y = 0.0f;
        bezos->velocity.y = 0.0f;
    }

    bezosUpdateColliderPos(bezos);
}

void bezosRender(struct Bezos* bezos, struct SpotLight* spotLights, int spotLightCount, struct RenderScene* renderScene) {
    if (!(bezos->flags & (BezosFlagsActive | BezosFlagsWaking))) {
        return;
    }

    Mtx* matrix = renderStateRequestMatrices(renderScene->renderState, 1);

    transformToMatrixL(&bezos->transform, matrix, SCENE_SCALE);

    Mtx* armature = renderStateRequestMatrices(renderScene->renderState, GHOSTJEFF_DEFAULT_BONES_COUNT);
    skCalculateTransforms(&bezos->armature, armature);

    struct LightConfiguration lightConfig;

    spotLightsFindConfiguration(spotLights, spotLightCount, &bezos->transform.position, 0.0f, &lightConfig);

    Light* light = spotLightsSetupLight(&lightConfig, &bezos->transform.position, renderScene->renderState);

    renderSceneAdd(renderScene, ghostjeff_model_gfx, matrix, GHOSTJEFF_BODY_INDEX, &bezos->transform.position, armature, light);
}

int bezosIsActive(struct Bezos* bezos) {
    return bezos->flags & (BezosFlagsActive | BezosFlagsWaking) && !(bezos->flags & BezosFlagsCaughtPlayer);
}
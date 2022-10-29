#include "item_requester.h"

#include "../build/assets/models/ui/item_prompt.h"
#include "../build/assets/materials/static.h"

#include "../build/assets/models/portal.h"

#include "../collision/collision_scene.h"

#include "../defs.h"

#include "../util/time.h"

#include "../math/mathf.h"

#include "item_render.h"

#define OSCILATE_PERIOD     4.0f
#define OSCILATE_HEIGHT     0.15f

#define ITEM_ANIMATE_DELAY    0.5f

#define ITEM_DROP_COYOTE_TIME   1.0f

void itemRequesterInit(struct ItemRequester* requester, struct ItemRequesterDefinition* definition) {
    requester->transform.position = definition->position;
    requester->transform.rotation = definition->rotation;
    requester->transform.scale = gOneVec;

    requester->requestedType = ItemTypeCount;
    requester->timeLeft = 0.0f;
    requester->duration = 0.0f;

    requester->requestDelay = 3.5f;

    collisionCapsuleInit(&requester->collisionCapsule, 1.0f, 0.5f);
    requester->collisionCapsule.center = requester->transform.position;
    collisionCapsuleUpdateBB(&requester->collisionCapsule);

    collisionSceneAddStatic(&gCollisionScene, &requester->collisionCapsule.collisionObject);

    skAnimatorInit(&requester->animator, PORTAL_DEFAULT_BONES_COUNT, NULL, NULL);
    skArmatureInit(&requester->armature, portal_model_gfx, PORTAL_DEFAULT_BONES_COUNT, portal_default_bones, portal_bone_parent, 0);
}

int itemRequesterUpdate(struct ItemRequester* requester, float timeScale) {
    int didFail = 0;

    skAnimatorUpdate(&requester->animator, requester->armature.boneTransforms, 1.0f);

    if (!skAnimatorIsRunning(&requester->animator)) {
        skAnimatorRunClip(&requester->animator, &portal_animations[PORTAL_PORTAL_PORTALSLOWLOOP_INDEX], SKAnimatorFlagsLoop);
    }

    if (requester->timeLeft > 0.0f) {
        requester->timeLeft -= FIXED_DELTA_TIME * timeScale;

        if (requester->timeLeft <= 0.0f) {
            requester->timeLeft = 0.0f;
            requester->requestedType = ItemTypeCount;
            didFail = 1;
        }
    }

    if (requester->requestDelay > 0.0f) {
        requester->requestDelay -= FIXED_DELTA_TIME;

        if (requester->requestDelay < 0.0f) {
            requester->requestDelay = 0.0f;
        }
    }

    itemUpdateAnimations(requester->requestedType);

    return didFail;
}

void itemRequesterRequestItem(struct ItemRequester* requester, enum ItemType itemType, float duration) {
    requester->duration = duration;
    requester->timeLeft = duration + ITEM_DROP_COYOTE_TIME;
    requester->requestedType = itemType;
}

int itemRequesterIsActive(struct ItemRequester* requester) {
    return requester->timeLeft > 0.0f || requester->requestDelay > 0.0f;
}

void itemRequesterRenderGenerate(struct ItemRequester* requester, int itemIndex, struct RenderState* renderState) {
    if (requester->requestedType < ItemTypeCount) {
        itemRenderGenerate(itemIndex, requester->requestedType, MAX((requester->timeLeft - ITEM_DROP_COYOTE_TIME) / requester->duration, 0.0f), requester->duration, renderState);
    }
}

void itemRequesterRender(struct ItemRequester* requester, int itemIndex, struct RenderScene* renderScene) {
    Mtx* mtx = renderStateRequestMatrices(renderScene->renderState, 1);
    transformToMatrixL(&requester->transform, mtx, SCENE_SCALE);

    Mtx* armature = renderStateRequestMatrices(renderScene->renderState, PORTAL_DEFAULT_BONES_COUNT);
    skCalculateTransforms(&requester->armature, armature);

    renderSceneAdd(renderScene, portal_model_gfx, mtx, ITEMS_EMMISIVE_INDEX, &requester->transform.position, armature, NULL);

    if (requester->timeLeft <= 0.0f) {
        return;
    }

    Mtx* matrix = renderStateRequestMatrices(renderScene->renderState, 1);

    struct Transform signTransform;
    signTransform.position = requester->transform.position;
    signTransform.position.y += OSCILATE_HEIGHT + OSCILATE_HEIGHT * sinf(requester->timeLeft * (M_PI * 2.0f / OSCILATE_PERIOD));
    signTransform.rotation = renderScene->cameraTransform.rotation;
    signTransform.scale = gOneVec;

    if (requester->flags & ItemRequesterFlagsHover) {
        vector3Scale(&signTransform.scale, &signTransform.scale, 1.25f);
    }

    float timeLeft = requester->timeLeft;

    if (timeLeft > ITEM_DROP_COYOTE_TIME) {
        float timeSinceStart = requester->duration + ITEM_DROP_COYOTE_TIME - timeLeft;

        if (timeSinceStart < ITEM_DROP_COYOTE_TIME) {
            timeLeft = timeSinceStart;
        } else {
            timeLeft = ITEM_DROP_COYOTE_TIME;
        }
    }

    if (timeLeft < ITEM_DROP_COYOTE_TIME) {
        vector3Scale(
            &signTransform.scale, 
            &signTransform.scale, 
            mathfEaseIn(timeLeft * (1.0f / ITEM_DROP_COYOTE_TIME), 3.0f)
        );
    }

    transformToMatrixL(&signTransform, matrix, SCENE_SCALE);

    Gfx* gfx = itemRenderUseImage(itemIndex, renderScene->renderState, ui_item_prompt_model_gfx);

    renderSceneAdd(renderScene, gfx, matrix, ITEM_PROMPT_INDEX, &requester->transform.position, NULL, NULL);
}

int itemRequesterHover(struct ItemRequester* requester, struct Item* item, struct Vector3* dropAt) {
    struct Vector3 offset;
    vector3Sub(&requester->transform.position, dropAt, &offset);
    offset.y = 0.0f;

    int wasThrown = (item->flags & ITEM_FLAGS_THROWN) != 0;

    int result = vector3MagSqrd(&offset) < (wasThrown ? ITEM_THROW_RADIUS * ITEM_THROW_RADIUS : ITEM_DROP_PICKUP_RADIUS * ITEM_DROP_PICKUP_RADIUS);

    if (result && item && requester->requestedType == item->type) {
        requester->flags |= ItemRequesterFlagsHover;
    } else {
        requester->flags &= ~ItemRequesterFlagsHover;
    }

    return result;
}

enum ItemDropResult itemRequesterDrop(struct ItemRequester* requester, struct Item* item, struct Vector3* dropAt) {
    if (itemRequesterHover(requester, item, dropAt)) {
        // thrown items cant trigger a wrong drop
        if (item->type != requester->requestedType && (item->flags & ITEM_FLAGS_THROWN)) {
            return ItemDropResultNone;
        }

        enum ItemDropResult result = item->type == requester->requestedType ? ItemDropResultSuccess : ItemDropResultFail;

        if (result == ItemDropResultSuccess) {
            itemSuccess(item, &requester->transform.position);
            skAnimatorRunClip(&requester->animator, &portal_animations[PORTAL_PORTAL_PORTAL_ARMATURE_PORTALITEMDROPPED_INDEX], 0);
        } else {
            itemDrop(item);
        }

        requester->timeLeft = 0.0;
        requester->requestedType = ItemTypeCount;
        
        return result;
    }

    return ItemDropResultNone;
}
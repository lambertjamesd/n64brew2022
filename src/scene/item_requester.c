#include "item_requester.h"

#include "../build/assets/models/ui/item_prompt.h"
#include "../build/assets/materials/static.h"

#include "../build/assets/models/portal.h"

#include "../collision/collision_scene.h"

#include "../defs.h"

#include "../util/time.h"

#include "../math/mathf.h"

#include "item_render.h"

#define OSCILATE_PERIOD     2.0f
#define OSCILATE_HEIGHT     0.25f

#define ITEM_ANIMATE_DELAY    0.5f

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
}

void itemRequesterUpdate(struct ItemRequester* requester) {
    if (requester->timeLeft > 0.0f) {
        requester->timeLeft -= FIXED_DELTA_TIME;

        if (requester->timeLeft <= 0.0f) {
            requester->timeLeft = 0.0f;
            requester->requestedType = ItemTypeCount;
        }
    }

    if (requester->requestDelay > 0.0f) {
        requester->requestDelay -= FIXED_DELTA_TIME;

        if (requester->requestDelay < 0.0f) {
            requester->requestDelay = 0.0f;
        }
    }
}

void itemRequesterRequestItem(struct ItemRequester* requester, enum ItemType itemType, float duration) {
    requester->duration = duration;
    requester->timeLeft = duration;
    requester->requestedType = itemType;
}

int itemRequesterIsActive(struct ItemRequester* requester) {
    return requester->timeLeft > 0.0f || requester->requestDelay > 0.0f;
}

void itemRequesterRender(struct ItemRequester* requester, struct RenderScene* renderScene) {
    Mtx* mtx = renderStateRequestMatrices(renderScene->renderState, 1);
    transformToMatrixL(&requester->transform, mtx, SCENE_SCALE);
    renderSceneAdd(renderScene, portal_model_gfx, mtx, ITEMS_EMMISIVE_INDEX, &requester->transform.position, NULL, NULL);

    if (requester->timeLeft <= 0.0f) {
        return;
    }

    Mtx* matrix = renderStateRequestMatrices(renderScene->renderState, 1);

    struct Transform signTransform;
    signTransform.position = requester->transform.position;
    signTransform.position.y += OSCILATE_HEIGHT + OSCILATE_HEIGHT * sinf(gTimePassed * (M_PI * 2.0f / OSCILATE_PERIOD));
    signTransform.rotation = renderScene->cameraTransform.rotation;
    signTransform.scale = gOneVec;

    if (requester->flags & ItemRequesterFlagsHover) {
        vector3Scale(&signTransform.scale, &signTransform.scale, 1.25f);
    }

    transformToMatrixL(&signTransform, matrix, SCENE_SCALE);

    Gfx* gfx = itemRenderUseImage(requester->requestedType, renderScene->renderState, ui_item_prompt_model_gfx);

    renderSceneAdd(renderScene, gfx, matrix, ITEM_PROMPT_INDEX, &requester->transform.position, NULL, NULL);
}

int itemRequesterHover(struct ItemRequester* requester, struct Item* item, struct Vector3* dropAt) {
    struct Vector3 offset;
    vector3Sub(&requester->transform.position, dropAt, &offset);
    offset.y = 0.0f;

    int result = vector3MagSqrd(&offset) < ITEM_DROP_PICKUP_RADIUS * ITEM_DROP_PICKUP_RADIUS;

    if (result && item && requester->requestedType == item->type) {
        requester->flags |= ItemRequesterFlagsHover;
    } else {
        requester->flags &= ~ItemRequesterFlagsHover;
    }

    return result;
}

enum ItemDropResult itemRequesterDrop(struct ItemRequester* requester, struct Item* item, struct Vector3* dropAt) {
    if (itemRequesterHover(requester, item, dropAt)) {
        enum ItemDropResult result = item->type == requester->requestedType ? ItemDropResultSuccess : ItemDropResultFail;

        if (result == ItemDropResultSuccess) {
            itemSuccess(item);
        } else {
            itemDrop(item);
        }

        requester->timeLeft = 0.0;
        requester->requestedType = ItemTypeCount;
        
        return result;
    }

    return ItemDropResultNone;
}
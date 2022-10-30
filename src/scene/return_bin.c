#include "return_bin.h"

#include "../build/assets/models/return_bin.h"
#include "../build/assets/materials/static.h"
#include "../collision/collision_scene.h"
#include "../defs.h"

#include "../math/mathf.h"
#include "../util/time.h"

#define ANIMATE_TIME   0.75f
#define ANIMATE_DEPTH   1.5f

void returnBinInit(struct ReturnBin* returnBin, struct ReturnBinDefinition* definition) {
    returnBin->position = definition->position;

    collisionCapsuleInit(&returnBin->collisionCapsule, 1.0f, 0.5f);
    returnBin->collisionCapsule.center = returnBin->position;
    collisionCapsuleUpdateBB(&returnBin->collisionCapsule);

    collisionSceneAddStatic(&gCollisionScene, &returnBin->collisionCapsule.collisionObject);

    returnBin->dropTime = 0.0f;
}

int returnBinDropItem(struct ReturnBin* returnBin, struct Item* item, struct Vector3* dropAt) {
    if (returnBinHover(returnBin, dropAt, NULL)) {
        itemReturn(item, &returnBin->position);
        returnBin->dropTime = ANIMATE_TIME;
        return 1;
    }

    return 0;
}

int returnBinHover(struct ReturnBin* returnBin, struct Vector3* dropAt, struct Vector3* hoverOutput) {
    struct Vector3 offset;
    vector3Sub(&returnBin->position, dropAt, &offset);
    offset.y = 0.0f;

    if (vector3MagSqrd(&offset) < ITEM_PICKUP_RADIUS * ITEM_PICKUP_RADIUS) {
        if (hoverOutput) {
            *hoverOutput = returnBin->position;
            hoverOutput->y += 0.5f;
        }
        return 1;
    }

    return 0;
}

void returnBinUpdate(struct ReturnBin* returnBin) {
    returnBin->dropTime = mathfMoveTowards(returnBin->dropTime, 0.0f, FIXED_DELTA_TIME);
}

void returnBinRender(struct ReturnBin* returnBin, struct RenderScene* renderScene) {
    Mtx* mtx = renderStateRequestMatrices(renderScene->renderState, 1);
    struct Transform transform;
    quatIdent(&transform.rotation);
    transform.position = returnBin->position;
    vector3Scale(&gOneVec, &transform.scale, 1.0f - ANIMATE_DEPTH * mathfBounceBackLerp(returnBin->dropTime * (1.0f / ANIMATE_TIME)));
    transformToMatrixL(&transform, mtx, SCENE_SCALE);
    renderSceneAdd(renderScene, return_bin_model_gfx, mtx, RETURN_BIN_INDEX, &returnBin->position, NULL, NULL);

}
#include "return_bin.h"

#include "../build/assets/models/return_bin.h"
#include "../build/assets/materials/static.h"
#include "../collision/collision_scene.h"
#include "../defs.h"

void returnBinInit(struct ReturnBin* returnBin, struct ReturnBinDefinition* definition) {
    returnBin->position = definition->position;

    collisionCapsuleInit(&returnBin->collisionCapsule, 1.0f, 0.5f);
    returnBin->collisionCapsule.center = returnBin->position;
    collisionCapsuleUpdateBB(&returnBin->collisionCapsule);

    collisionSceneAddStatic(&gCollisionScene, &returnBin->collisionCapsule.collisionObject);
}

int returnBinDropItem(struct ReturnBin* returnBin, struct Item* item, struct Vector3* dropAt) {
    if (returnBinHover(returnBin, dropAt, NULL)) {
        itemReturn(item, &returnBin->position);
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

void returnBinRender(struct ReturnBin* returnBin, struct RenderScene* renderScene) {
    Mtx* mtx = renderStateRequestMatrices(renderScene->renderState, 1);
    guTranslate(mtx, returnBin->position.x * SCENE_SCALE, returnBin->position.y * SCENE_SCALE, returnBin->position.z * SCENE_SCALE);
    renderSceneAdd(renderScene, return_bin_model_gfx, mtx, DEFAULT_INDEX, &returnBin->position, NULL, NULL);

}
#include "conveyor.h"

#include "../build/assets/models/conveyor.h"
#include "../build/assets/materials/static.h"

#include "../models/models.h"
#include "../util/time.h"

#include "../defs.h"

#define BELT_SPEED  1.0f
#define BELT_OFFSET_GAP     1.0f

#define START_BELT_OFFSET   3.0f

struct Vector3 gBeltEnd = {0.0f, 0.3f, 0.7f};

void conveyorInit(struct Conveyor* conveyor, struct ConveyorDefinition* definition) {
    conveyor->transform.position = definition->position;
    conveyor->transform.rotation = definition->rotation;
    conveyor->transform.scale = gOneVec;   
}

void conveyorUpdate(struct Conveyor* conveyor) {
    struct Transform transform;

    quatIdent(&transform.rotation);
    transform.scale = gOneVec;

    for (int i = 0; i < 2; ++i) {
        if (!conveyor->pendingItems[i]) {
            break;
        }

        struct Vector3 beltOffset = gBeltEnd;
        beltOffset.z -= conveyor->beltOffset[i];

        transformPoint(&conveyor->transform, &beltOffset, &transform.position);

        itemUpdateTarget(conveyor->pendingItems[i], &transform);

        conveyor->beltOffset[i] -= BELT_SPEED * FIXED_DELTA_TIME;

        if (conveyor->beltOffset[i] < i * BELT_OFFSET_GAP) {
            conveyor->beltOffset[i] = i * BELT_OFFSET_GAP;
        }
    }
}

void conveyorRender(struct Conveyor* conveyor, struct RenderScene* renderScene) {
    Mtx* mtx = renderStateRequestMatrices(renderScene->renderState, 1);
    transformToMatrixL(&conveyor->transform, mtx, SCENE_SCALE);

    renderSceneAdd(renderScene, conveyor_model_gfx, mtx, FURNITURE_WAREHOUSE_INDEX, &conveyor->transform.position, NULL);
}

int conveyorCanAcceptItem(struct Conveyor* conveyor) {
    return conveyor->pendingItems[1] == NULL && (conveyor->pendingItems[0] == NULL || conveyor->beltOffset[0] == 0.0f);
}

void conveyorAcceptItem(struct Conveyor* conveyor, struct Item* item) {
    int newIndex = conveyor->pendingItems[0] ? 1 : 0;

    conveyor->pendingItems[newIndex] = item;
    conveyor->beltOffset[newIndex] = START_BELT_OFFSET;

    quatIdent(&item->transform.rotation);
    item->transform.scale = gOneVec;
    struct Vector3 beltOffset = gBeltEnd;
    beltOffset.z -= START_BELT_OFFSET;
    transformPoint(&conveyor->transform, &beltOffset, &item->transform.position);
    item->flags = ITEM_FLAGS_ATTACHED;
}

struct Item* conveyorReleaseItem(struct Conveyor* conveyor) {
    if (!conveyor->pendingItems[0]) {
        return NULL;
    }

    struct Item* result = conveyor->pendingItems[0];

    conveyor->pendingItems[0] = conveyor->pendingItems[1];
    conveyor->beltOffset[0] = conveyor->beltOffset[1];

    conveyor->pendingItems[1] = NULL;
    conveyor->beltOffset[1] = 0.0f;

    itemMarkNewTarget(result);

    return result;
}
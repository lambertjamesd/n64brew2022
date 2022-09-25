#include "table.h"

#include "../util/memory.h"

#include "../build/assets/models/table.h"
#include "../build/assets/materials/static.h"

#include "../defs.h"

struct Vector3 gTable0Slots[] = {
    {0.5f, 0.6f, 0.0f},
    {-0.5f, 0.6f, 0.0f},
};

struct TableType gTableTypes[] = {
    {table_model_gfx, gTable0Slots, sizeof(gTable0Slots) / sizeof(*gTable0Slots), FURNITURE_WAREHOUSE_INDEX},
};

void tableInit(struct Table* table, struct TableDefinition* def) {
    table->position = def->position;
    table->tableType = &gTableTypes[def->tableType];
    table->itemSlots = malloc(sizeof(struct Item*) * table->tableType->itemSlotCount);

    for (int i = 0; i < table->tableType->itemSlotCount; ++i) {
        table->itemSlots[i] = NULL;
    }
}

void tableRender(struct Table* table, struct RenderScene* renderScene) {
    Mtx* matrix = renderStateRequestMatrices(renderScene->renderState, 1);
    guTranslate(matrix, table->position.x * SCENE_SCALE, table->position.y * SCENE_SCALE, table->position.z * SCENE_SCALE);

    renderSceneAdd(
        renderScene,
        table->tableType->displayList,
        matrix,
        table->tableType->materialIndex,
        &table->position,
        NULL,
        NULL
    );
}

struct Item* tablePickupItem(struct Table* table, struct Vector3* grabFrom) {
    for (int i = 0; i < table->tableType->itemSlotCount; ++i) {
        if (!table->itemSlots[i]) {
            continue;
        }

        struct Vector3 itemPosition;
        vector3Add(&table->position, &table->tableType->itemSlots[i], &itemPosition);

        struct Vector3 offset;
        vector3Sub(&itemPosition, grabFrom, &offset);
        offset.y = 0.0f;

        if (vector3MagSqrd(&offset) < ITEM_PICKUP_RADIUS * ITEM_PICKUP_RADIUS) {
            struct Item* result = table->itemSlots[i];
            table->itemSlots[i] = NULL;
            itemMarkNewTarget(result);
            return result;
        }
    }

    return NULL;
}

int tableDropItem(struct Table* table, struct Item* item, struct Vector3* dropAt) {
    for (int i = 0; i < table->tableType->itemSlotCount; ++i) {
        if (table->itemSlots[i]) {
            continue;
        }

        struct Vector3 itemPosition;
        vector3Add(&table->position, &table->tableType->itemSlots[i], &itemPosition);

        struct Vector3 offset;
        vector3Sub(&itemPosition, dropAt, &offset);
        offset.y = 0.0f;

        if (vector3MagSqrd(&offset) < ITEM_PICKUP_RADIUS * ITEM_PICKUP_RADIUS) {
            table->itemSlots[i] = item;
            itemMarkNewTarget(item);
            struct Transform transform;
            transform.position = itemPosition;
            quatIdent(&transform.rotation);
            transform.scale = gOneVec;

            itemUpdateTarget(item, &transform);

            return 1;
        }
    }

    return 0;
}
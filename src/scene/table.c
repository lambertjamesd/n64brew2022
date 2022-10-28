#include "table.h"

#include "../util/memory.h"

#include "../build/assets/models/table.h"
#include "../build/assets/models/table_vertical.h"
#include "../build/assets/materials/static.h"

#include "../collision/collision_scene.h"

#include "../defs.h"

struct FightingPair {
    enum ItemType attacker;
    enum ItemType attacked;
};

#define ITEM_FLAG(itemType) (1 << (itemType))

struct FightingPair gFightingPairs[] = {
    {ItemTypeCat, ItemTypeRat},
    {ItemTypeScarecrow, ItemTypeCrow},
};

struct TableType* gTableTypes[] = {
    [TABLE_HORIZONTAL] = &table_definition,
    [TABLE_VERTICAL] = &table_vertical_definition,
};

int tableUpdate(struct Table* table) {
    if ((table->flags & TABLE_FLAGS_NEED_TO_CHECK_ATTACKS) == 0) {
        return 0;
    }

    int itemFlags = 0;

    for (int i = 0; i < table->tableType->itemSlotCount; ++i) {
        if (table->itemSlots[i]) {
            if (!(table->itemSlots[i]->flags & ITEM_FLAGS_ATTACHED)) {
                return 0;
            }

            itemFlags |= ITEM_FLAG(table->itemSlots[i]->type);
        }
    }

    table->flags &= ~TABLE_FLAGS_NEED_TO_CHECK_ATTACKS;

    int attacker = 0;
    int attacked = 0;

    for (int i = 0; i < sizeof(gFightingPairs) / sizeof(*gFightingPairs); ++i) {
        int attackerFlags = ITEM_FLAG(gFightingPairs[i].attacker);
        int attackedFlags = ITEM_FLAG(gFightingPairs[i].attacked);
        
        if ((itemFlags & (attackerFlags | attackedFlags)) == (attackerFlags | attackedFlags)) {
            attacker |= attackerFlags;
            attacked |= attackedFlags;
        }
    }

    if (!attacker || !attacked) {
        return itemFlags != 0;
    }

    struct Vector3 attackPosition;

    for (int i = 0; i < table->tableType->itemSlotCount; ++i) {
        if (table->itemSlots[i] && (ITEM_FLAG(table->itemSlots[i]->type) & attacked) != 0) {
            itemAttacked(table->itemSlots[i]);
            attackPosition = table->itemSlots[i]->target.position;
            table->itemSlots[i] = NULL;
        }
    }

    for (int i = 0; i < table->tableType->itemSlotCount; ++i) {
        if (table->itemSlots[i] && (ITEM_FLAG(table->itemSlots[i]->type) & attacker) != 0) {
            itemAttack(table->itemSlots[i], &attackPosition);
        }
    }

    return itemFlags != 0;
}

void tableInit(struct Table* table, struct TableDefinition* def) {
    table->position = def->position;
    table->tableType = gTableTypes[def->tableType];
    table->itemSlots = malloc(sizeof(struct Item*) * table->tableType->itemSlotCount);

    for (int i = 0; i < table->tableType->itemSlotCount; ++i) {
        table->itemSlots[i] = NULL;
    }

    box3DOffset(&table->tableType->boundingBox, &table->position, &table->collisionObject.boundingBox);

    table->collisionObject.data = &table->collisionObject;
    table->collisionObject.minkowskiSum = collisionObjectBoundingBox;
    table->collisionObject.flags = 0;
    table->flags = 0;

    collisionSceneAddStatic(&gCollisionScene, &table->collisionObject);
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

int tableFindSlot(struct Table* table, struct Vector3* grabFrom, int filterEmpty, int filterFilled) {
    int result = -1;
    float resultDistance = 0.0f;

    for (int i = 0; i < table->tableType->itemSlotCount; ++i) {
        if (filterEmpty && !table->itemSlots[i]) {
            continue;
        }

        if (filterFilled && table->itemSlots[i]) {
            continue;
        }

        struct Vector3 itemPosition;
        vector3Add(&table->position, &table->tableType->itemSlots[i], &itemPosition);

        struct Vector3 offset;
        vector3Sub(&itemPosition, grabFrom, &offset);
        offset.y = 0.0f;

        float distanceSqrd = vector3MagSqrd(&offset);

        if (distanceSqrd < ITEM_PICKUP_RADIUS * ITEM_PICKUP_RADIUS) {
            if (result == -1 || resultDistance > distanceSqrd) {
                resultDistance = distanceSqrd;
                result = i;
            }
        }
    }

    return result;
}

struct Item* tablePickupItem(struct Table* table, struct Vector3* grabFrom) {
    int resultIndex = tableFindSlot(table, grabFrom, 1, 0);

    if (resultIndex == -1) {
        return NULL;
    }

    struct Item* result = table->itemSlots[resultIndex];
    table->itemSlots[resultIndex] = NULL;
    itemMarkNewTarget(result);
    return result;
}

int tableDropItem(struct Table* table, struct Item* item, struct Vector3* dropAt) {
    int resultIndex = tableFindSlot(table, dropAt, 0, 1);

    if (resultIndex == -1) {
        return 0;
    }

    if (table->itemSlots[resultIndex]) {
        itemMarkNewTarget(table->itemSlots[resultIndex]);
    }

    struct Vector3 itemPosition;
    vector3Add(&table->position, &table->tableType->itemSlots[resultIndex], &itemPosition);

    table->itemSlots[resultIndex] = item;
    itemMarkNewTarget(item);
    struct Transform transform;
    transform.position = itemPosition;
    quatIdent(&transform.rotation);
    transform.scale = gOneVec;

    itemUpdateTarget(item, &transform);

    table->flags |= TABLE_FLAGS_NEED_TO_CHECK_ATTACKS;

    return 1;
}

int tableHoverItem(struct Table* table, struct Vector3* dropAt, struct Vector3* hoverOutput) {
    int resultIndex = tableFindSlot(table, dropAt, 0, 0);

    if (resultIndex == -1) {
        return 0;
    }

    struct Vector3 itemPosition;
    vector3Add(&table->position, &table->tableType->itemSlots[resultIndex], &itemPosition);
    *hoverOutput = itemPosition;
    return 1;
}

int tableSwapItem(struct Table* table, struct Item* item, struct Vector3* dropAt, struct Item** replacement) {
    int resultIndex = tableFindSlot(table, dropAt, 0, 0);

    if (resultIndex == -1 || !table->itemSlots[resultIndex]) {
        return 0;
    }

    *replacement = table->itemSlots[resultIndex];
    itemMarkNewTarget(table->itemSlots[resultIndex]);

    if (table->itemSlots[resultIndex]) {
        itemMarkNewTarget(table->itemSlots[resultIndex]);
    }

    struct Vector3 itemPosition;
    vector3Add(&table->position, &table->tableType->itemSlots[resultIndex], &itemPosition);

    table->itemSlots[resultIndex] = item;
    itemMarkNewTarget(item);
    struct Transform transform;
    transform.position = itemPosition;
    quatIdent(&transform.rotation);
    transform.scale = gOneVec;

    itemUpdateTarget(item, &transform);

    table->flags |= TABLE_FLAGS_NEED_TO_CHECK_ATTACKS;

    return 1;
}
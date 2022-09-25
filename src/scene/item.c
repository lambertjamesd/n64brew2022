#include "item.h"

#include "../util/time.h"
#include "../util/memory.h"

#include "../build/assets/materials/static.h"
#include "../build/assets/models/pumpkin.h"
#include "../build/assets/models/conveyor.h"
#include "../defs.h"

#define MAX_SNAP_SPEED      4.0f

#define DROP_GRAVITY        -9.8f

#define POOF_TIME           0.5f
#define POOF_DAMPING        0.95f
#define POOF_SCALING        1.05f

struct ItemTypeDefinition gItemDefinitions[ItemTypeCount] = {
    [ItemTypePumpkin] = {
        pumpkin_model_gfx,
        ITEMS_INDEX,
        0,
        0,
        NULL,
        NULL,
    },
};

void itemInit(struct Item* item, enum ItemType itemType, struct Transform* initialPose) {
    item->next = NULL;
    item->type = itemType;
    item->transform = *initialPose;
    item->target = *initialPose;
    item->flags = ITEM_FLAGS_ATTACHED;
    
    struct ItemTypeDefinition* definition = &gItemDefinitions[itemType];

    if (definition->boneCount) {
        item->flags |= ITEM_FLAGS_HAS_ARMATURE;
        
        skArmatureInit(
            &item->armature, 
            definition->dl, 
            definition->boneCount, 
            definition->defaultBones, 
            definition->boneParent, 
            definition->attachmentCount
        );

        skAnimatorInit(&item->animator, definition->boneCount, NULL, NULL);
    }
}

void itemUpdate(struct Item* item) {
    if (item->flags & ITEM_FLAGS_GONE) {
        return;
    }

    if (item->flags & ITEM_FLAGS_HAS_ARMATURE) {
        skAnimatorUpdate(&item->animator, item->armature.boneTransforms, 1.0f);
    }

    if (item->flags & ITEM_FLAGS_POOFED) {
        vector3Scale(&item->dropInfo.velocity, &item->dropInfo.velocity, POOF_DAMPING);
        vector3AddScaled(&item->transform.position, &item->dropInfo.velocity, FIXED_DELTA_TIME, &item->transform.position);
        vector3Scale(&item->transform.scale, &item->transform.scale, POOF_SCALING);

        item->dropInfo.pooftimer -= FIXED_DELTA_TIME;

        if (item->dropInfo.pooftimer < 0.0f) {
            item->flags |= ITEM_FLAGS_GONE;
        }

    } else if (item->flags & ITEM_FLAGS_DROPPED) {
        item->dropInfo.velocity.y += DROP_GRAVITY * FIXED_DELTA_TIME;

        vector3AddScaled(&item->transform.position, &item->dropInfo.velocity, FIXED_DELTA_TIME, &item->transform.position);

        if (item->transform.position.y < 0.0f) {
            item->transform.position.y = 0.0f;
            item->dropInfo.velocity.x = 0.0f;
            item->dropInfo.velocity.y = -item->dropInfo.velocity.y;
            item->dropInfo.velocity.z = 0.0f;
            item->flags |= ITEM_FLAGS_POOFED;
            item->dropInfo.pooftimer = POOF_TIME;
        }
    } else if (item->flags & ITEM_FLAGS_ATTACHED) {
        item->transform = item->target;
    } else {
        if (vector3MoveTowards(&item->transform.position, &item->target.position, MAX_SNAP_SPEED * FIXED_DELTA_TIME, &item->transform.position)) {
            item->transform.rotation = item->target.rotation;
            item->flags |= ITEM_FLAGS_ATTACHED;
        } else {
            quatLerp(&item->transform.rotation, &item->target.rotation, 0.1f, &item->transform.rotation);
        }
    }
}

void itemRender(struct Item* item, Light* light, struct RenderScene* renderScene) {
    struct ItemTypeDefinition* definition = &gItemDefinitions[item->type];

    Mtx* mtx = renderStateRequestMatrices(renderScene->renderState, 1);

    transformToMatrixL(&item->transform, mtx, SCENE_SCALE);

    Mtx* armature = NULL;

    if (definition->boneCount) {
        armature = renderStateRequestMatrices(renderScene->renderState, definition->boneCount);
        skCalculateTransforms(&item->armature, armature);
    }

    Gfx* gfx = definition->dl;
    int material = definition->materialIndex;

    if (item->flags & ITEM_FLAGS_POOFED) {
        material = WHITE_SMOKE_INDEX;

        Gfx* gfx = renderStateAllocateDLChunk(renderScene->renderState, 4);
        Gfx* dl = gfx;

        float lerp = item->dropInfo.pooftimer * (1.0f / POOF_TIME);

        gDPSetBlendColor(dl++, 255, 255, 255, 255 * lerp);
        gDPSetPrimColor(dl++, 255, 255, 255, 255, 255, 255 * lerp);
        gSPDisplayList(dl++, definition->dl);
        gSPEndDisplayList(dl++);
    }

    renderSceneAdd(renderScene, gfx, mtx, material, &item->transform.position, armature, light);
}

void itemUpdateTarget(struct Item* item, struct Transform* transform) {
    item->target = *transform;
}

void itemMarkNewTarget(struct Item* item) {
    item->flags &= ~ITEM_FLAGS_ATTACHED;
}

void itemDrop(struct Item* item) {
    item->flags |= ITEM_FLAGS_DROPPED;

    item->dropInfo.velocity = gZeroVec;
    item->dropInfo.pooftimer = 0.0f;
}

void itemPoolInit(struct ItemPool* itemPool) {
    itemPool->itemHead = NULL;
    itemPool->unusedHead = NULL;
    itemPool->itemCount = 0;
}

struct Item* itemPoolNew(struct ItemPool* itemPool, enum ItemType itemType, struct Transform* initialPose) {
    struct Item* result = NULL;

    if (itemPool->unusedHead) {
        result = itemPool->unusedHead;
        itemPool->unusedHead = itemPool->unusedHead->next;
    } else {
        result = malloc(sizeof(struct Item));
    }

    result->next = itemPool->itemHead;
    itemPool->itemHead = result;

    ++itemPool->itemCount;

    return result;
}

void itemPoolFree(struct ItemPool* itemPool, struct Item* item) {
    struct Item* prev = NULL;
    struct Item* current = itemPool->itemHead;

    while (current != NULL && current != item) {
        prev = current;
        current = current->next;
    }

    if (!current) {
        return;
    }

    if (prev) {
        prev->next = current->next;
    } else {
        itemPool->itemHead = current->next;
    }

    item->next = itemPool->unusedHead;
    itemPool->unusedHead = item;

    --itemPool->itemCount;
}

void itemPoolUpdate(struct ItemPool* itemPool) {
    struct Item* current = itemPool->itemHead;
    struct Item* prev = NULL;

    while (current != NULL) {
        itemUpdate(current);

        struct Item* next = current->next;

        if (current->flags & ITEM_FLAGS_GONE) {
            prev->next = next;

            current->next = itemPool->unusedHead;
            itemPool->unusedHead = current;
        } else {
            prev = current;
        }

        current = next;
    }
}

void itemPoolRender(struct ItemPool* itemPool, struct SpotLight* spotLights, int spotLightCount, struct RenderScene* renderScene) {
    struct Item* current = itemPool->itemHead;

    struct LightConfiguration* configurations = stackMalloc(sizeof(struct LightConfiguration) * itemPool->itemCount);
    struct LightConfiguration* currentConfiguration = configurations;

    while (current != NULL) {
        spotLightsFindConfiguration(spotLights, spotLightCount, &current->transform.position, NULL, currentConfiguration);
        Light* light = spotLightsSetupLight(currentConfiguration, &current->transform.position, renderScene->renderState);
        itemRender(current, light, renderScene);

        current = current->next;
        ++currentConfiguration;
    }

    stackMallocFree(configurations);
}
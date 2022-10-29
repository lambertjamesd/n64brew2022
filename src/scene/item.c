#include "item.h"

#include "../util/time.h"
#include "../util/memory.h"

#include "../build/assets/materials/static.h"
#include "../build/assets/models/pumpkin.h"
#include "../build/assets/models/hat.h"
#include "../build/assets/models/brain.h"
#include "../build/assets/models/broom.h"
#include "../build/assets/models/candle.h"
#include "../build/assets/models/cat.h"
#include "../build/assets/models/cobweb.h"
#include "../build/assets/models/crow.h"
#include "../build/assets/models/hand.h"
#include "../build/assets/models/rat.h"
#include "../build/assets/models/scarecrow.h"
#include "../build/assets/models/skull.h"
#include "../build/assets/models/spider.h"
#include "../build/assets/models/conveyor.h"
#include "../defs.h"

#define MAX_SNAP_SPEED      4.0f

#define DROP_GRAVITY        -9.8f

#define POOF_TIME           1.5f
#define POOF_DAMPING        0.92f
#define POOF_SCALING        1.01f

#define TRANSLATE_SPEED     2.0f

#define THROW_HORZ_VELOCITY 2.5f
#define THROW_VERTICAL_VELOCITY 3.5f

#define ATTACKED_DELAY      1.5f

struct ItemTypeDefinition gItemDefinitions[ItemTypeCount] = {
    [ItemTypePumpkin] = {
        pumpkin_model_gfx,
        ITEMS_INDEX,
        0,
        0,
        NULL,
        NULL,
        &pumpkin_camera,
        &pumpkin_light,
        NULL,
        NULL,
        .grabTransform = &pumpkin_grab_transform,
    },
    [ItemTypeHat] = {
        hat_model_gfx,
        ITEMS_DOUBLE_SIDED_INDEX,
        0,
        0,
        NULL,
        NULL,
        &hat_camera,
        &hat_light,
        NULL,
        NULL,
        .grabTransform = &hat_grab_transform,
    },
    [ItemTypeBrain] = {
        brain_model_gfx,
        ITEMS_INDEX,
        0,
        0,
        NULL,
        NULL,
        &brain_camera,
        &brain_light,
        NULL,
        NULL,
        .grabTransform = &brain_grab_transform,
    },
    [ItemTypeBroom] = {
        broom_model_gfx,
        ITEMS_INDEX,
        0,
        0,
        NULL,
        NULL,
        &broom_camera,
        &broom_light,
        NULL,
        NULL,
        .grabTransform = &broom_grab_transform,
        .useTransform = &broom_use_transform,
    },
    [ItemTypeCandle] = {
        candle_model_gfx,
        ITEMS_INDEX,
        0,
        0,
        NULL,
        NULL,
        &candle_camera,
        &candle_light,
        NULL,
        NULL,
        .grabTransform = &candle_grab_transform,
    },
    [ItemTypeCat] = {
        cat_model_gfx,
        ITEMS_INDEX,
        CAT_DEFAULT_BONES_COUNT,
        CAT_ATTACHMENT_COUNT,
        cat_default_bones,
        cat_bone_parent,
        &cat_camera,
        &cat_light,
        &cat_animations[CAT_CAT_CAT_ARMATURE_CATIDLE_INDEX],
        &cat_animations[CAT_CAT_CAT_ARMATURE_CATHISS_INDEX],
        .grabTransform = &cat_grab_transform,
    },
    [ItemTypeCobweb] = {
        cobweb_model_gfx,
        ITEMS_INDEX,
        0,
        0,
        NULL,
        NULL,
        &cobweb_camera,
        &cobweb_light,
        NULL,
        NULL,
        .grabTransform = &cat_grab_transform,
    },
    [ItemTypeCrow] = {
        crow_model_gfx,
        ITEMS_INDEX,
        CROW_DEFAULT_BONES_COUNT,
        CROW_ATTACHMENT_COUNT,
        crow_default_bones,
        crow_bone_parent,
        &crow_camera,
        &crow_light,
        NULL,
        NULL,
        .grabTransform = &crow_grab_transform,
    },
    [ItemTypeHand] = {
        hand_model_gfx,
        RATHAND_INDEX,
        HAND_DEFAULT_BONES_COUNT,
        HAND_ATTACHMENT_COUNT,
        hand_default_bones,
        hand_bone_parent,
        &hand_camera,
        &hand_light,
        &hand_animations[HAND_HAND__HAND_0_HANDWALK_INDEX],
        NULL,
        .grabTransform = &hand_grab_transform,
    },
    [ItemTypeRat] = {
        rat_model_gfx,
        RATHAND_INDEX,
        RAT_DEFAULT_BONES_COUNT,
        RAT_ATTACHMENT_COUNT,
        rat_default_bones,
        rat_bone_parent,
        &rat_camera,
        &rat_light,
        &rat_animations[RAT_RAT_RATIDLE_INDEX],
        NULL,
        .grabTransform = &rat_grab_transform,
    },
    [ItemTypeScarecrow] = {
        scarecrow_model_gfx,
        ITEMS_INDEX,
        0,
        0,
        NULL,
        NULL,
        &scarecrow_camera,
        &scarecrow_light,
        NULL,
        NULL,
        .grabTransform = &scarecrow_grab_transform,
    },
    [ItemTypeSkull] = {
        skull_model_gfx,
        ITEMS_INDEX,
        0,
        0,
        NULL,
        NULL,
        &skull_camera,
        &skull_light,
        NULL,
        NULL,
        .grabTransform = &skull_grab_transform,
    },
    [ItemTypeSpider] = {
        spider_model_gfx,
        ITEMS_INDEX,
        SPIDER_DEFAULT_BONES_COUNT,
        SPIDER_ATTACHMENT_COUNT,
        spider_default_bones,
        spider_bone_parent,
        &spider_camera,
        &spider_light,
        &spider_animations[SPIDER_SPIDER_SPIDERIDLE_INDEX],
        .grabTransform = &spider_grab_transform,
    },
};

struct ItemIdleAnimators gIdleAnimators[ItemTypeCount];

void itemInitIdleAnimators() {
    for (int i = 0; i < ItemTypeCount; ++i) {
        struct ItemTypeDefinition* definition = &gItemDefinitions[i];
        struct ItemIdleAnimators* animator = &gIdleAnimators[i];

        if (!definition->boneCount) {
            continue;
        }

        skArmatureInit(
            &animator->armature, 
            definition->dl, 
            definition->boneCount, 
            definition->defaultBones, 
            definition->boneParent, 
            definition->attachmentCount
        );

        skAnimatorInit(&animator->animator, definition->boneCount, NULL, NULL);

        if (definition->idleAnimation) {
            skAnimatorRunClip(&animator->animator, definition->idleAnimation, SKAnimatorFlagsLoop);
        }

        animator->mtxArmature = NULL;
    }
}

void itemMarkNeedsUpdate() {
    for (int i = 0; i < ItemTypeCount; ++i) {
        gIdleAnimators[i].hasUpdated = 0;
    }
}

void itemMarkNeedsRender() {
    for (int i = 0; i < ItemTypeCount; ++i) {
        gIdleAnimators[i].mtxArmature = NULL;
    }
}

void itemUpdateAnimations(enum ItemType itemType) {
    struct ItemTypeDefinition* definition = &gItemDefinitions[itemType];
    struct ItemIdleAnimators* animator = &gIdleAnimators[itemType];

    if (!definition->boneCount || animator->hasUpdated) {
        return;
    }

    skAnimatorUpdate(&animator->animator, animator->armature.boneTransforms, 1.0f);
    animator->hasUpdated = 1;
}

Mtx* itemGetIdle(enum ItemType itemType, struct RenderState* renderState) {
    struct ItemTypeDefinition* definition = &gItemDefinitions[itemType];
    struct ItemIdleAnimators* animator = &gIdleAnimators[itemType];

    if (!definition->boneCount) {
        return NULL;
    }

    if (animator->mtxArmature) {
        return animator->mtxArmature;
    }

    animator->mtxArmature = renderStateRequestMatrices(renderState, definition->boneCount);
    skCalculateTransforms(&animator->armature, animator->mtxArmature);

    return animator->mtxArmature;
}

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

void itemDestroy(struct Item* item) {
    skAnimatorCleanup(&item->animator);
}

void itemUpdate(struct Item* item) {
    if (item->flags & ITEM_FLAGS_GONE) {
        return;
    }

    // itemUpdateAnimations(item->type);
    
    if (item->flags & ITEM_FLAGS_HAS_ARMATURE) {
        skAnimatorUpdate(&item->animator, item->armature.boneTransforms, 1.0f);
    }

    if (item->flags & ITEM_FLAGS_ATTACKED) {
        item->attackedInfo.attackedDelayTimer -= FIXED_DELTA_TIME;

        if (item->attackedInfo.attackedDelayTimer < 0.0f) {
            item->flags &= ~ITEM_FLAGS_ATTACKED;
            item->flags |= ITEM_FLAGS_POOFED;
            item->dropInfo.velocity = gZeroVec;
            item->dropInfo.velocity.y = 1.0f;
            item->dropInfo.pooftimer = POOF_TIME;
        }
    } else if (item->flags & ITEM_FLAGS_POOFED) {
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

        if (item->flags & (ITEM_FLAGS_SUCCESS | ITEM_FLAGS_RETURNED)) {
            struct Vector3 targetHorizontal = item->target.position;
            targetHorizontal.y = item->transform.position.y;
            vector3MoveTowards(&item->transform.position, &targetHorizontal, FIXED_DELTA_TIME * TRANSLATE_SPEED, &item->transform.position);
        }

        if (item->transform.position.y < 0.0f) {
            item->transform.position.y = 0.0f;
            item->dropInfo.velocity.x = 0.0f;
            item->dropInfo.velocity.y = -item->dropInfo.velocity.y * 0.5f;
            item->dropInfo.velocity.z = 0.0f;

            if (item->flags & (ITEM_FLAGS_SUCCESS | ITEM_FLAGS_RETURNED)) {
                item->flags |= ITEM_FLAGS_GONE;
            } else {
                item->flags |= ITEM_FLAGS_POOFED;
                item->dropInfo.pooftimer = POOF_TIME;
            }
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

void itemPreRender(struct Item* item, struct RenderState* renderState) {
    item->mtxTransform = renderStateRequestMatrices(renderState, 1);
    transformToMatrixL(&item->transform, item->mtxTransform, SCENE_SCALE);

    if ((item->flags & ITEM_FLAGS_HAS_ARMATURE) && skAnimatorIsRunning(&item->animator)) {
        item->mtxArmature = renderStateRequestMatrices(renderState, item->animator.boneCount);
        skCalculateTransforms(&item->armature, item->mtxArmature);
    } else {
        item->mtxArmature = itemGetIdle(item->type, renderState);
    }
}

void itemRender(struct Item* item, Light* light, struct RenderScene* renderScene) {
    struct ItemTypeDefinition* definition = &gItemDefinitions[item->type];

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

    renderSceneAdd(renderScene, gfx, item->mtxTransform, material, &item->transform.position, item->mtxArmature, light);
}

void itemUpdateTarget(struct Item* item, struct Transform* transform) {
    item->target = *transform;
}

void itemMarkNewTarget(struct Item* item) {
    item->flags &= ~(ITEM_FLAGS_ATTACHED | ITEM_FLAGS_DROPPED);
}

void itemDrop(struct Item* item) {
    item->flags |= ITEM_FLAGS_DROPPED;

    item->dropInfo.velocity = gZeroVec;
    item->dropInfo.pooftimer = 0.0f;
}

void itemThrow(struct Item* item, struct Vector3* horizontalDir) {
    item->flags |= ITEM_FLAGS_DROPPED | ITEM_FLAGS_THROWN;

    item->dropInfo.velocity = *horizontalDir;
    item->dropInfo.velocity.y = 0.0f;

    vector3Normalize(&item->dropInfo.velocity, &item->dropInfo.velocity);
    vector3Scale(&item->dropInfo.velocity, &item->dropInfo.velocity, THROW_HORZ_VELOCITY);
    item->dropInfo.velocity.y = THROW_VERTICAL_VELOCITY;

    item->dropInfo.pooftimer = 0.0f;
}

void itemAttacked(struct Item* item) {
    item->flags |= ITEM_FLAGS_ATTACKED;
    item->attackedInfo.attackedDelayTimer = ATTACKED_DELAY;
}

void itemAttack(struct Item* item, struct Vector3* target) {
    struct Vector3 offset;
    vector3Sub(target, &item->target.position, &offset);
    offset.y = 0.0f;

    quatLook(&offset, &gUp, &item->target.rotation);
    item->flags &= ~ITEM_FLAGS_ATTACHED;

    struct ItemTypeDefinition* definition = &gItemDefinitions[item->type];

    if (definition->attackAnimation) {
        skAnimatorRunClip(&item->animator, definition->attackAnimation, 0);
    }
}

void itemSuccess(struct Item* item, struct Vector3* portalAt) {
    item->flags |= ITEM_FLAGS_DROPPED | ITEM_FLAGS_SUCCESS;
    item->dropInfo.velocity = gZeroVec;
    item->dropInfo.pooftimer = 0.0f;
    item->target.position = *portalAt;
}

void itemReturn(struct Item* item, struct Vector3* binAt) {
    item->flags |= ITEM_FLAGS_DROPPED | ITEM_FLAGS_RETURNED;
    item->dropInfo.velocity = gZeroVec;
    item->dropInfo.pooftimer = 0.0f;
    item->target.position = *binAt;

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

    ++itemPool->itemCount;
    
    itemInit(result, itemType, initialPose);

    result->next = itemPool->itemHead;
    itemPool->itemHead = result;

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

    itemDestroy(item);

    item->next = itemPool->unusedHead;
    itemPool->unusedHead = item;

    --itemPool->itemCount;
}

enum ItemPoolUpdateResult itemPoolUpdate(struct ItemPool* itemPool, struct Tutorial* tutorial, struct Vector3* itemPos, SceneDropCallback dropCallback, struct Scene* scene) {
    struct Item* current = itemPool->itemHead;
    struct Item* prev = NULL;

    int hadFailure = 0;
    int hadThrowFailure = 0;
    int hadSuccess = 0;

    while (current != NULL) {
        itemUpdate(current);

        struct Item* next = current->next;

        if (current->flags & ITEM_FLAGS_GONE) {
            if (current->flags & ITEM_FLAGS_SUCCESS) {
                if (current->flags & ITEM_FLAGS_THROWN_SUCCESS) {
                    tutorialItemDropped(tutorial, TutorialDropTypeSuccessThrow);
                } else {
                    tutorialItemDropped(tutorial, TutorialDropTypeSuccess);
                }
                hadSuccess = 1;
            } else if (!(current->flags & ITEM_FLAGS_RETURNED)) {
                tutorialItemDropped(tutorial, TutorialDropTypeFail);
                hadFailure = 1;
                if (current->flags & ITEM_FLAGS_THROWN) {
                    hadThrowFailure = 1;
                }
                *itemPos = current->transform.position;
            }
            
            if (prev) {
                prev->next = next;
            } else {
                itemPool->itemHead = next;
            }

            current->next = itemPool->unusedHead;
            itemPool->unusedHead = current;
        } else {
            if (current->flags & ITEM_FLAGS_THROWN && current->dropInfo.velocity.y < 0.0f) {
                if (dropCallback(scene, current, &current->transform.position)) {
                    current->flags &= ~ITEM_FLAGS_THROWN;
                    current->flags |= ITEM_FLAGS_THROWN_SUCCESS;
                }
            }

            prev = current;
        }

        current = next;
    }

    if (hadSuccess) {
        return ItemPoolUpdateResultSuccess;
    }
    
    if (hadFailure) {
        return hadThrowFailure ? ItemPoolUpdateResultFailThrow : ItemPoolUpdateResultFail;
    }

    return ItemPoolUpdateResultNone;
}

void itemPoolRender(struct ItemPool* itemPool, struct SpotLight* spotLights, int spotLightCount, struct RenderScene* renderScene) {
    struct Item* current = itemPool->itemHead;

    struct LightConfiguration* configurations = stackMalloc(sizeof(struct LightConfiguration) * itemPool->itemCount);
    struct LightConfiguration* currentConfiguration = configurations;

    while (current != NULL) {
        spotLightsFindConfiguration(spotLights, spotLightCount, &current->transform.position, 0.0f, currentConfiguration);
        Light* light = spotLightsSetupLight(currentConfiguration, &current->transform.position, renderScene->renderState);
        itemRender(current, light, renderScene);

        current = current->next;
        ++currentConfiguration;
    }

    stackMallocFree(configurations);
}
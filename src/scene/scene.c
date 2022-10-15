#include "scene.h"

#include "../defs.h"

#include "../graphics/copycb.h"
#include "../models/models.h"
#include "../materials/basic_lit.h"
#include "../materials/toon_lit.h"
#include "../materials/outline_pass.h"
#include "../materials/point_light_rendered.h"
#include "../controls/controller.h"
#include "../util/time.h"
#include "../defs.h"
#include "../level/level.h"
#include "../graphics/render_scene.h"
#include "../graphics/pallete_operations.h"
#include "../util/memory.h"
#include "../math/mathf.h"
#include "../collision/collision_scene.h"
#include "item_render.h"
#include "table_surface.h"
#include "shadow_volume_group.h"
#include "../ui/spritefont.h"
#include "../ui/sprite.h"
#include "../ui/nightchilde.h"

#include "../build/assets/materials/ui.h"
#include "../build/assets/materials/static.h"
#include "../build/assets/materials/pallete.h"

#define ROTATE_PER_SECOND       (M_PI * 0.25f)
#define MOVE_PER_SECOND         (3.0f)
#define MIN_DISTANCE            (2.0f)
#define MAX_DISTANCE            (20.0f)

#define RENDER_SCENE_CAPACITY   256

Lights1 gLights = gdSPDefLights1(0x10, 0, 0, 0xE0, 0xE0, 0xE0, 90, 90, 0);

u16 __attribute__((aligned(64))) gPlayerShadowBuffers[MAX_PLAYERS][SHADOW_MAP_WIDTH * SHADOW_MAP_HEIGHT];

#define LIGHT_ORBIT_RADIUS  (5.0f)
#define LIGHT_ORBIT_PERIOD  3.0f

#define DROPS_PER_FULL_BAR          5
#define TIME_TO_DECAY_FULL_BAR      120

void materialSetBasicLit(struct RenderState* renderState, int objectIndex) {
    gSPDisplayList(renderState->dl++, gBasicLitMaterial);
}

void materialSetToon(struct RenderState* renderState, int objectIndex) {
    toonLitUse(renderState, 4 + 4 * objectIndex, 2);
}

void materialSetOutline(struct RenderState* renderState, int objectIndex) {
    gSPDisplayList(renderState->dl++, gOutlinePass);
}

#define GROUND_LERP  TEXEL0, 0, ENVIRONMENT, PRIMITIVE, 0, 0, 0, ENVIRONMENT


void sceneInit(struct Scene* scene, struct LevelDefinition* definition, int playerCount) {
    itemPoolInit(&scene->itemPool);
    collisionSceneInit(&gCollisionScene, definition->tableCount + playerCount + definition->boundaryCount + definition->conveyorCount + definition->itemRequesterCount);
    
    itemCoordinatorInit(&scene->itemCoordinator, definition->script);

    cameraInit(
        &scene->camera, 
        definition->cameraDefinition.verticalFov, 
        definition->cameraDefinition.nearPlane * SCENE_SCALE,
        definition->cameraDefinition.farPlane * SCENE_SCALE
    );

    scene->camera.transform.position = definition->cameraDefinition.position;
    scene->camera.transform.rotation = definition->cameraDefinition.rotation;

    scene->playerCount = (u8)playerCount;
    for (int i = 0; i < playerCount; ++i) {
        playerInit(&scene->players[i], &definition->playerStart[i], i, gPlayerShadowBuffers[i]);
    }

    scene->itemSlotCount = definition->itemSlotCount;
    scene->itemSlots = malloc(sizeof(struct ItemSlot) * scene->itemSlotCount);
    for (int i = 0; i < scene->itemSlotCount; ++i) {
        itemSlotInit(&scene->itemSlots[i], &definition->itemSlots[i]);
    }

    scene->spotLightCount = definition->spotLightCount;
    scene->spotLights = malloc(sizeof(struct SpotLight) * scene->spotLightCount);
    for (int i = 0; i < scene->spotLightCount; ++i) {
        spotLightInit(&scene->spotLights[i], &definition->spotLights[i], &scene->camera.transform.position);
    }

    scene->conveyorCount = definition->conveyorCount;
    scene->conveyors = malloc(sizeof(struct Conveyor) * scene->conveyorCount);
    for (int i = 0; i < scene->conveyorCount; ++i) {
        conveyorInit(&scene->conveyors[i], &definition->conveyors[i]);
    }

    scene->tableCount = definition->tableCount;
    scene->tables = malloc(sizeof(struct Table) * scene->tableCount);
    for (int i = 0; i < scene->tableCount; ++i) {
        tableInit(&scene->tables[i], &definition->tables[i]);
    }

    scene->itemRequesterCount = definition->itemRequesterCount;
    scene->itemRequesters = malloc(sizeof(struct ItemRequester) * scene->itemRequesterCount);
    for (int i = 0; i < scene->itemRequesterCount; ++i) {
        itemRequesterInit(&scene->itemRequesters[i], &definition->itemRequesters[i]);
    }

    scene->dropPenalty = 0.0f;

    struct CollisionBoundary* boundaries = malloc(sizeof(struct CollisionBoundary) * definition->boundaryCount);
    for (int i = 0; i < definition->boundaryCount; ++i) {
        collisionBoundaryInit(&boundaries[i], &definition->boundary[i].a, &definition->boundary[i].b);
        collisionSceneAddStatic(&gCollisionScene, &boundaries[i].collisionObject);
    }

    bezosInit(&scene->bezos);
}

unsigned ignoreInputFrames = 10;

void sceneUpdate(struct Scene* scene) {
    if (ignoreInputFrames) {
        --ignoreInputFrames;
    }

    for (int i = 0; i < scene->playerCount; ++i) {
        struct Player* player = &scene->players[i];

        playerUpdate(player);

        struct Vector3 grabFrom;
        playerGrabPoint(player, &grabFrom);

        player->hoverLocation.y = -1.0f;
        if (player->holdingItem) {
            sceneItemHover(scene, player->holdingItem, &grabFrom, &player->hoverLocation);
        }

        int didReplace = 0;

        if (controllerGetButtonDown(i, B_BUTTON | A_BUTTON) && player->holdingItem) {
            struct Item* replacement = NULL;
            if (sceneSwapItem(scene, player->holdingItem, &grabFrom, &replacement)) {
                player->holdingItem = NULL;
                playerHandObject(player, replacement);
                didReplace = 1;
            }
        }

        if (!didReplace && controllerGetButtonDown(i, B_BUTTON) && player->holdingItem) {
            if (!sceneDropItem(scene, player->holdingItem, &grabFrom)) {
                struct Item* item = player->holdingItem;

                itemDrop(player->holdingItem);

                scene->dropPenalty += 1.0f / DROPS_PER_FULL_BAR;

                if (scene->dropPenalty > 1.0f) {
                    scene->dropPenalty = 1.0f;
                }

                if (mathfRandomFloat() < scene->dropPenalty) {
                    bezosActivate(&scene->bezos, &item->transform.position);
                }
            }

            player->holdingItem = NULL;
        }

        if (!didReplace && playerCanGrab(player) && controllerGetButtonDown(i, A_BUTTON)) {
            struct Item* item = scenePickupItem(scene, &grabFrom);

            if (item) {
                playerHandObject(player, item);
            }
        }
    }

    bezosUpdate(&scene->bezos);

    for (int i = 0; i < scene->conveyorCount; ++i) {
        if (conveyorCanAcceptItem(&scene->conveyors[i])) {
            enum ItemType newItemType = itemCoordinatorNextArrival(&scene->itemCoordinator);

            if (newItemType < ItemTypeCount) {
                struct Item* newItem = itemPoolNew(&scene->itemPool, newItemType, &scene->conveyors[i].transform);
                conveyorAcceptItem(&scene->conveyors[i], newItem);
            }
        }

        conveyorUpdate(&scene->conveyors[i]);
    }

    itemPoolUpdate(&scene->itemPool);

    for (int i = 0; i < scene->spotLightCount; ++i) {
        spotLightUpdate(&scene->spotLights[i], &scene->camera.transform.position);
    }

    int activeRequesterCount = 0;

    for (int i = 0; i < scene->itemRequesterCount; ++i) {
        if (itemRequesterIsActive(&scene->itemRequesters[i])) {
            ++activeRequesterCount;
        }
    }

    for (int i = 0; i < scene->itemRequesterCount; ++i) {
        if (!itemRequesterIsActive(&scene->itemRequesters[i])) {
            enum ItemType newItemType = itemCoordinatorNextRequest(&scene->itemCoordinator, activeRequesterCount);

            if (newItemType < ItemTypeCount) {
                itemRequesterRequestItem(&scene->itemRequesters[i], newItemType, itemCoordinatorTimeout(&scene->itemCoordinator));
                ++activeRequesterCount;
            }
        }
        
        itemRequesterUpdate(&scene->itemRequesters[i]);
    }

    if (scene->dropPenalty > 0.0f) {
        scene->dropPenalty -= FIXED_DELTA_TIME / TIME_TO_DECAY_FULL_BAR;

        if (scene->dropPenalty <= 0.0f) {
            scene->dropPenalty = 0.0f;
            bezosDeactivate(&scene->bezos);
        }
    }

    itemCoordinatorUpdate(&scene->itemCoordinator);

    collisionSceneCollide(&gCollisionScene);
}

struct Colorf32 gAmbientLight = {0.0f, 0.2f, 0.4f, 255};
struct Colorf32 gAmbientScale = {0.5f, 0.5f, 0.5f, 255};
struct Colorf32 gLightColor = {0.3f, 0.3f, 0.15f, 255};

struct Plane gGroundPlane = {{0.0f, 1.0f, 0.0}, -0.05f};

void sceneRender(struct Scene* scene, struct RenderState* renderState, struct GraphicsTask* task) {
    for (int i = 0; i < DEFAULT_UI_INDEX; ++i) {
        spriteSetLayer(renderState, i, ui_material_list[i]);
    }

    Mtx* identity = renderStateRequestMatrices(renderState, 1);
    guMtxIdent(identity);
    gSPMatrix(renderState->dl++, identity, G_MTX_LOAD | G_MTX_NOPUSH | G_MTX_MODELVIEW);

    itemRenderGenerateAll(renderState);

    struct LightConfiguration playerLightConfig[scene->playerCount];

    for (int i = 0; i < scene->playerCount; ++i) {
        playerSetupTransforms(&scene->players[i], renderState);
    }

    struct Item* currentItem = scene->itemPool.itemHead;

    while (currentItem) {
        itemPreRender(currentItem, renderState);

        currentItem = currentItem->next;
    }

    for (int i = 0; i < scene->playerCount; ++i) {
        struct Player* player = &scene->players[i];

        struct Vector3 capsuleCenter;
        transformPoint(&player->transform, &player->shadowMap.offset, &capsuleCenter);

        spotLightsFindConfiguration(scene->spotLights, scene->spotLightCount, &capsuleCenter, player->shadowMap.subjectRadius, &playerLightConfig[i]);

        struct Vector3 lightPosition;

        if (spotLightsGetPosition(&playerLightConfig[i], &lightPosition)) {  
            Gfx* playerShadowGfx = playerGenerateShadowMapGfx(&scene->players[i], renderState);

            shadowMapRender(
                &player->shadowMap, 
                renderState, 
                task, 
                &lightPosition, 
                &player->transform, 
                playerShadowGfx
            );

            player->shadowMap.flags |= SHADOW_MAP_ENABLED;
        } else {
            player->shadowMap.flags &= ~SHADOW_MAP_ENABLED;
        }
    }

    gDPPipeSync(renderState->dl++);
    gDPSetScissor(renderState->dl++, G_SC_NON_INTERLACE, 0, 0, SCREEN_WD, SCREEN_HT);
    gSPViewport(renderState->dl++, &fullscreenViewport);
    gSPSetGeometryMode(renderState->dl++, G_ZBUFFER);
    gSPMatrix(renderState->dl++, identity, G_MTX_LOAD | G_MTX_NOPUSH | G_MTX_MODELVIEW);

    gDPPipeSync(renderState->dl++);
    gDPSetColorImage(renderState->dl++, G_IM_FMT_CI, G_IM_SIZ_8b, SCREEN_WD, indexColorBuffer);
    gDPSetCycleType(renderState->dl++, G_CYC_FILL);
    gDPSetFillColor(renderState->dl++, 0x02020202);
    gDPFillRectangle(renderState->dl++, 0, 0, SCREEN_WD-1, SCREEN_HT-1);
    gDPPipeSync(renderState->dl++);
    gDPSetCycleType(renderState->dl++, G_CYC_1CYCLE);

    struct FrustrumCullingInformation cullingInformation;

    cameraSetupMatrices(&scene->camera, renderState, (float)SCREEN_WD / (float)SCREEN_HT, &cullingInformation);
    
    gSPSetLights1(renderState->dl++, gLights);
    gDPSetRenderMode(renderState->dl++, G_RM_OPA_SURF, G_RM_OPA_SURF2);
    gSPClearGeometryMode(renderState->dl++, G_ZBUFFER);

    // render ground
    struct RenderScene* renderScene = renderSceneNew(&scene->camera.transform, renderState, RENDER_SCENE_CAPACITY, ~0, 0);

    for (unsigned i = 0; i < gCurrentLevel->groundContentCount; ++i) {
        renderSceneAdd(renderScene, gCurrentLevel->groundContent[i].displayList, NULL, gCurrentLevel->groundContent[i].materialIndex, &gZeroVec, NULL, NULL);
    }

    renderSceneGenerate(renderScene, renderState);
    renderSceneFree(renderScene);

    gDPPipeSync(renderState->dl++);
    gDPSetRenderMode(renderState->dl++, G_RM_ZB_OPA_SURF, G_RM_ZB_OPA_SURF2);
    gSPSetGeometryMode(renderState->dl++, G_ZBUFFER);

    struct ShadowVolumeGroup shadowGroup;
    shadowVolumeGroupInit(&shadowGroup);
    
    // render objects
    renderScene = renderSceneNew(&scene->camera.transform, renderState, RENDER_SCENE_CAPACITY, ~0, levelMaterialDefault());

    for (unsigned i = 0; i < gCurrentLevel->staticContentCount; ++i) {
        renderSceneAdd(renderScene, gCurrentLevel->staticContent[i].displayList, NULL, gCurrentLevel->staticContent[i].materialIndex, &gZeroVec, NULL, NULL);
    }

    for (unsigned i = 0; i < scene->playerCount; ++i) {
        struct Player* player = &scene->players[i];

        struct ShadowVolumeTarget target;
        Light* light = spotLightsSetupLight(&playerLightConfig[i], &player->transform.position, renderState);
        playerToShadowTarget(player, &target, light);

        struct CollisionCapsule shadowCapsule;
        collisionCapsuleInit(&shadowCapsule, 0.0f, player->shadowMap.subjectRadius);
        transformPoint(&player->transform, &player->shadowMap.offset, &shadowCapsule.center);
        collisionCapsuleUpdateBB(&shadowCapsule);

        target.collisionObject = &shadowCapsule.collisionObject;

        Light* firstLight = shadowVolumeGroupPopulate(&shadowGroup, scene->spotLights, scene->spotLightCount, &target);
        
        playerRender(&scene->players[i], firstLight, renderScene);
    }

    for (unsigned i = 0; i < scene->spotLightCount; ++i) {
        spotLightRender(&scene->spotLights[i], renderScene);
    }

    bezosRender(&scene->bezos, scene->spotLights, scene->spotLightCount, renderScene);

    itemPoolRender(&scene->itemPool, scene->spotLights, scene->spotLightCount, renderScene);

    for (unsigned i = 0; i < scene->conveyorCount; ++i) {
        conveyorRender(&scene->conveyors[i], renderScene);
    }

    for (unsigned i = 0; i < scene->tableCount; ++i) {
        tableRender(&scene->tables[i], renderScene);
    }

    for (int i = 0; i < scene->itemRequesterCount; ++i) {
        itemRequesterRender(&scene->itemRequesters[i], renderScene);
    }

    renderSceneGenerate(renderScene, renderState);
    renderSceneFree(renderScene);

    // render shadows

    for (unsigned i = 0; i < scene->playerCount; ++i) {
        shadowMapRenderOntoPlane(&scene->players[i].shadowMap, renderState, &gGroundPlane);
    }

    gDPPipeSync(renderState->dl++);
    gSPDisplayList(renderState->dl++, levelMaterial(SHADOW_INDEX));

    for (unsigned tableIndex = 0; tableIndex < scene->tableCount; ++tableIndex) {
        struct Box3D tableBB;

        struct TableType* tableType = scene->tables[tableIndex].tableType;

        vector3Add(&tableType->boundingBox.min, &scene->tables[tableIndex].position, &tableBB.min);
        vector3Add(&tableType->boundingBox.max, &scene->tables[tableIndex].position, &tableBB.max);

        struct Vector3 weightedCenter = gZeroVec;
        float totalWeight = 0.0f;
        
        for (unsigned lightIndex = 0; lightIndex < scene->spotLightCount; ++lightIndex) {
            struct SpotLight* spotLight = &scene->spotLights[lightIndex];

            if (!box3DHasOverlap(&tableBB, &spotLight->boundingBox)) {
                continue;
            }

            struct Vector3 boxPoint;
            box3DNearestPoint(&tableBB, &spotLight->rigidBody.transform.position, &boxPoint);

            float weight = spotLightClosenessWeight(spotLight, &boxPoint);

            if (weight > 0) {
                totalWeight += weight;
                vector3AddScaled(&weightedCenter, &spotLight->rigidBody.transform.position, weight, &weightedCenter);
            }
        }

        if (totalWeight > 0.0f) {
            vector3Scale(&weightedCenter, &weightedCenter, 1.0f / totalWeight);
            tableSurfaceRenderShadow(&tableType->surfaceMesh, &scene->tables[tableIndex].position, &weightedCenter, renderState);
        }
    }

    // render lights

    gDPPipeSync(renderState->dl++);
    gDPSetRenderMode(renderState->dl++, G_RM_ZB_OPA_SURF, G_RM_ZB_OPA_SURF2);
    gSPSetGeometryMode(renderState->dl++, G_ZBUFFER);
    gDPSetAlphaCompare(renderState->dl++, G_AC_NONE);
    gDPSetBlendColor(renderState->dl++, 0, 0, 0, 0);
    gSPDisplayList(renderState->dl++, levelMaterial(ADDITIVE_LIGHT_INDEX));

    for (unsigned i = 0; i < scene->spotLightCount; ++i) {
        spotLightRenderProjection(&scene->spotLights[i], renderState);
    }


    gDPPipeSync(renderState->dl++);
    gSPDisplayList(renderState->dl++, levelMaterial(ADDITIVE_LIGHT_DECAL_INDEX));

    for (unsigned tableIndex = 0; tableIndex < scene->tableCount; ++tableIndex) {
        struct Box3D tableBB;

        struct TableType* tableType = scene->tables[tableIndex].tableType;

        vector3Add(&tableType->boundingBox.min, &scene->tables[tableIndex].position, &tableBB.min);
        vector3Add(&tableType->boundingBox.max, &scene->tables[tableIndex].position, &tableBB.max);
        
        for (unsigned lightIndex = 0; lightIndex < scene->spotLightCount; ++lightIndex) {
            if (!box3DHasOverlap(&tableBB, &scene->spotLights[lightIndex].boundingBox)) {
                continue;
            }

            tableSurfaceRenderLight(&tableType->surfaceMesh, &scene->tables[tableIndex].position, &scene->spotLights[lightIndex], renderState);
        }
    }

    gDPPipeSync(renderState->dl++);
    gSPGeometryMode(renderState->dl++, G_CULL_FRONT, G_CULL_BACK);
    gDPSetRenderMode(renderState->dl++, G_RM_ZB_OPA_SURF, G_RM_ZB_OPA_SURF2);

    struct Transform transform;
    transformInitIdentity(&transform);

    gDPPipeSync(renderState->dl++);
    gDPSetCycleType(renderState->dl++, G_CYC_1CYCLE);
    gDPSetColorImage(renderState->dl++, G_IM_FMT_RGBA, G_IM_SIZ_16b, SCREEN_WD, osVirtualToPhysical(task->framebuffer));
    gSPSegment(renderState->dl++, SOURCE_CB_SEGMENT, indexColorBuffer);

    enum PalleteEffects effects = 0;

    if (scene->bezos.flags & BezosFlagsActive) {
        effects |= PalleteEffectsGrayscaleRed;
    }

    u16* pallete = palleteGenerateLit(
        (struct Coloru8*)pallete_half_pallete_rgba_32b, 
        &gAmbientLight,
        &gAmbientScale, 
        &gLightColor, 
        effects,
        renderState
    );

    gDPLoadTLUT_pal256(renderState->dl++, pallete);

    gSPDisplayList(renderState->dl++, gCopyCB);

    gDPPipeSync(renderState->dl++);
    gSPDisplayList(renderState->dl++, ui_default_ui);

    spriteInit(renderState);

    // spriteDraw(renderState, NIGHTCHILDE_INDEX, 10, 10, 128, 64, 0, 0, 1, 1);

    fontRenderText(renderState, &gNightChilde, "aabb", 10, 10, 0);

    spriteFinish(renderState);

    // shadowMapRenderDebug(renderState, scene->players[0].shadowMap.buffer);
}

struct Item* scenePickupItem(struct Scene* scene, struct Vector3* grabFrom) {
    for (int i = 0; i < scene->conveyorCount; ++i) {
        struct Item* result = conveyorPickupItem(&scene->conveyors[i], grabFrom);

        if (result) {
            return result;
        }
    }

    for (int i = 0; i < scene->tableCount; ++i) {
        struct Item* result = tablePickupItem(&scene->tables[i], grabFrom);

        if (result) {
            return result;
        }
    }
    

    return NULL;
}

int sceneDropItem(struct Scene* scene, struct Item* item, struct Vector3* dropAt) {
    for (int i = 0; i < scene->itemRequesterCount; ++i) {
        enum ItemDropResult dropResult = itemRequesterDrop(&scene->itemRequesters[i], item, dropAt);
        if (dropResult) {
            if (dropResult == ItemDropResultSuccess) {
                itemCoordinatorMarkSuccess(&scene->itemCoordinator);
            }

            return 1;
        }
    }

    for (int i = 0; i < scene->tableCount; ++i) {
        if (tableDropItem(&scene->tables[i], item, dropAt)) {
            return 1;
        }
    }

    return 0;
}

int sceneSwapItem(struct Scene* scene, struct Item* item, struct Vector3* dropAt, struct Item** replacement) {
    for (int i = 0; i < scene->tableCount; ++i) {
        if (tableSwapItem(&scene->tables[i], item, dropAt, replacement)) {
            return 1;
        }
    }

    return 0;
}

int sceneItemHover(struct Scene* scene, struct Item* item, struct Vector3* dropAt, struct Vector3* hoverOutput) {
    for (int i = 0; i < scene->itemRequesterCount; ++i) {
        if (itemRequesterHover(&scene->itemRequesters[i], item, dropAt) && scene->itemRequesters[i].requestedType == item->type) {
            *hoverOutput = scene->itemRequesters[i].transform.position;
            hoverOutput->y += 0.5f;
            return 1;
        }
    }


    for (int i = 0; i < scene->tableCount; ++i) {
        int result = tableHoverItem(&scene->tables[i], dropAt, hoverOutput);
        if (result) {
            return result;
        }
    }

    return 0;
}
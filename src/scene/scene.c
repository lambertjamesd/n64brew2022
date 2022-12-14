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
#include "../util/rom.h"
#include "../menu/ui.h"
#include "../savefile/savefile.h"
#include "../menu/main_menu.h"

#include "../audio/soundplayer.h"

#include "../build/assets/materials/ui.h"
#include "../build/assets/materials/static.h"
#include "../build/assets/materials/pallete.h"
#include "../build/src/audio/clips.h"

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
#define TIME_TO_DECAY_FULL_BAR      40
#define TIME_TO_FILL_USE_BAR        140
#define USE_ITEM_CHECK_INTERVAL     7.0f
#define MIN_ITEM_SPAWN_THRESHOLD    0.3f

#define APPEAR_FLASH_PERIOD  0.1f
#define APPEAR_FLASH_COUNT   2

#define FADE_IN_DURATION    2.0f

#define RED_FLASH_TIME      0.75f

#define GREEN_FLASH_TIME    0.55f


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
    randomSeed((int)gTimePassed, (int)(13.0f * gTimePassed));

    itemPoolInit(&scene->itemPool);
    collisionSceneInit(&gCollisionScene, 
        definition->tableCount + 
        playerCount + 
        definition->boundaryCount + 
        definition->conveyorCount + 
        definition->itemRequesterCount +
        definition->returnBinCount +
        1 // bezos
    );
    
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

    scene->returnBinCount = definition->returnBinCount;
    scene->returnBins = malloc(sizeof(struct ReturnBin) * scene->returnBinCount);
    for (int i = 0; i < scene->returnBinCount; ++i) {
        returnBinInit(&scene->returnBins[i], &definition->returnBins[i]);
    }

    scene->dropPenalty = 0.0f;
    scene->appearTime = -10.0f;
    scene->fadeInTime = FADE_IN_DURATION;
    scene->currentLevelTime = 0.0f;
    scene->penaltyTime = 0.0f;
    scene->successTime = 0.0f;

    struct CollisionBoundary* boundaries = malloc(sizeof(struct CollisionBoundary) * definition->boundaryCount);
    for (int i = 0; i < definition->boundaryCount; ++i) {
        collisionBoundaryInit(&boundaries[i], &definition->boundary[i].a, &definition->boundary[i].b);
        collisionSceneAddStatic(&gCollisionScene, &boundaries[i].collisionObject);
    }

    bezosInit(&scene->bezos);
    tutorialInit(&scene->tutorial, definition->tutorial, definition->tutorialOnStart);
    endScreenInit(&scene->endScreen);
    itemInitIdleAnimators();
    pauseMenuInit(&scene->pauseMenu);

    scene->musicSound = definition->music;
    scene->musicId = SOUND_ID_NONE;
}

unsigned ignoreInputFrames = 10;

void sceneCheckSpawn(struct Scene* scene, struct Vector3* at, float minThreshold) {
    if (bezosIsActive(&scene->bezos)) {
        bezosSpeedUp(&scene->bezos);
        return;
    }

    if (scene->dropPenalty > minThreshold && mathfRandomFloat() < scene->dropPenalty) {
        bezosActivate(&scene->bezos, at);
        soundPlayerPlay(SOUNDS_GHOSTAPPEAR, 1.0f, 1.0f, NULL);
        scene->appearTime = gTimePassed;
        scene->dropPenalty = mathfMoveTowards(scene->dropPenalty, 1.0f, 0.5f);
        playerStopUsingItem(&scene->players[0]);
    }
}

void sceneApplyPenalty(struct Scene* scene, struct Vector3* at, int isThrown) {
    if (!tutorialIsImmune(&scene->tutorial)) {
        if (!bezosIsActive(&scene->bezos)) {
            scene->dropPenalty += isThrown ? 1.0f / DROPS_PER_FULL_BAR : 1.0f / DROPS_PER_FULL_BAR;

            if (scene->dropPenalty > 1.0f) {
                scene->dropPenalty = 1.0f;
            }
        }

        sceneCheckSpawn(scene, at, 0.0f);
        
        scene->penaltyTime = RED_FLASH_TIME;
    }
}

short sceneCurrentMusic(struct Scene* scene) {    
    if (pauseMenuIsPaused(&scene->pauseMenu)) {
        return SOUNDS_SUNNY_SUMMER_OF_SOUL_N_SACRIFICE;
    }

    if (scene->endScreen.success != EndScreenTypeNone) {
        if (scene->endScreen.success == EndScreenTypeFail) {
            return SOUNDS_IS_THIS_HELL_OR_SHOEGAZE;
        }

        return SOUNDS_JAH_SPOOKS;
    }

    if (bezosIsActive(&scene->bezos)) {
        return SOUNDS_TONYDANGER;
    }

    if (scene->tutorial.animationLerp) {
        return SOUNDS_TRICK_OR_TREAT;
    }

    short musicOveride = itemCoordinatorMusic(&scene->itemCoordinator);

    if (musicOveride != -1) {
        return musicOveride;
    }

    return scene->musicSound;
}

void sceneUpdate(struct Scene* scene) {
    if (ignoreInputFrames) {
        --ignoreInputFrames;
    }

    if (scene->fadeInTime > 0.0f) {
        scene->fadeInTime -= FIXED_DELTA_TIME;

        if (scene->fadeInTime < 0.0f) {
            scene->fadeInTime = 0.0f;
        }

        return;
    }

    float speed = 0.5f;
    short desiredMusic = sceneCurrentMusic(scene);
    short currentMusic;
    
    if (soundPlayerIsPlaying(scene->musicId)) {
        currentMusic = soundPlayerSoundClipId(scene->musicId);
    } else {
        currentMusic = -1;
    }

    if (desiredMusic != currentMusic) {
        soundPlayerStop(scene->musicId);
        if (desiredMusic != -1) {
            scene->musicId = soundPlayerPlay(desiredMusic, 1.0f, speed, NULL);
        } else {
            scene->musicId = -1;
        }
    }

    if (scene->endScreen.success == EndScreenTypeNone && pauseMenuUpdate(&scene->pauseMenu)) {
        return;
    }


    // allow the tutorial to pause
    if (tutorialUpdate(&scene->tutorial)) {
        return;
    }

    if (endScreenUpdate(&scene->endScreen)) {
        if (endScreenIsDone(&scene->endScreen)) {
            if (scene->endScreen.success == EndScreenTypeSuccessLastLevel) {
                mainMenuShowCredits(&gMainMenu);
            } else if (scene->endScreen.success == EndScreenTypeSuccess) {
                levelQueueLoad(NEXT_LEVEL);
            } else {
                levelQueueLoad(gCurrentLevelIndex);
            }
            return;
        }

        return;
    }

    itemMarkNeedsUpdate();

    int isEnding = scene->endScreen.success != EndScreenTypeNone;

    for (int i = 0; i < scene->playerCount; ++i) {
        struct Player* player = &scene->players[i];

        playerUpdate(player);

        if (isEnding) {
            continue;
        }

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
                soundPlayerPlay(SOUNDS_DROPITEM, 0.7f, 1.0f, NULL);
                itemDrop(player->holdingItem);
            }

            player->holdingItem = NULL;
        }

        if (!didReplace && playerCanGrab(player) && controllerGetButtonDown(i, A_BUTTON)) {
            struct Item* item = scenePickupItem(scene, &grabFrom);

            if (item) {
                playerHandObject(player, item);
            }
        }

        if (player->holdingItem && (player->holdingItem->flags & ITEM_FLAGS_ATTACHED) != 0) {
            tutorialItemPickedUp(&scene->tutorial);
        }
    }

    bezosUpdate(&scene->bezos, sceneNearestPlayerPos(scene));

    if (scene->bezos.flags & BezosFlagsCaughtPlayer && scene->endScreen.success == EndScreenTypeNone) {
        endScreenEndGame(&scene->endScreen, EndScreenTypeFail);

        for (int i = 0; i < scene->playerCount; ++i) {
            playerKill(&scene->players[i]);
        }
    }

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

    for (int i = 0; i < scene->returnBinCount; ++i) {
        returnBinUpdate(&scene->returnBins[i]);
    }

    struct Vector3 bezosSpawn;

    enum ItemPoolUpdateResult itemPoolResult = itemPoolUpdate(&scene->itemPool, &scene->tutorial, &bezosSpawn, sceneDropItem, scene);

    if (itemPoolResult == ItemPoolUpdateResultFail || itemPoolResult == ItemPoolUpdateResultFailThrow) {
        sceneApplyPenalty(scene, &bezosSpawn, itemPoolResult == ItemPoolUpdateResultFailThrow);
    } else if (itemPoolResult == ItemPoolUpdateResultSuccess) {
        scene->successTime = GREEN_FLASH_TIME;
    }

    for (int i = 0; i < scene->spotLightCount; ++i) {
        spotLightUpdate(&scene->spotLights[i], &scene->camera.transform.position);
    }

    for (int i = 0; i < scene->tableCount; ++i) {
        if (tableUpdate(&scene->tables[i])) {
            tutorialItemDropped(&scene->tutorial, TutorialDropTypeTable);
        }
    }

    int activeRequesterCount = 0;

    for (int i = 0; i < scene->itemRequesterCount; ++i) {
        if (itemRequesterIsActive(&scene->itemRequesters[i])) {
            ++activeRequesterCount;
        }
    }

    float requesterTime = bezosIsActive(&scene->bezos) ? 0.75f : 1.0f;

    for (int i = 0; i < scene->itemRequesterCount; ++i) {
        if (!itemRequesterIsActive(&scene->itemRequesters[i])) {
            enum ItemType newItemType = itemCoordinatorNextRequest(&scene->itemCoordinator, activeRequesterCount);

            if (newItemType < ItemTypeCount) {
                itemRequesterRequestItem(&scene->itemRequesters[i], newItemType, itemCoordinatorTimeout(&scene->itemCoordinator));
                ++activeRequesterCount;
            }
        }
        
        if (itemRequesterUpdate(&scene->itemRequesters[i], requesterTime)) {
            sceneApplyPenalty(scene, &scene->itemRequesters[i].transform.position, 0);
        }
    }

    if (playerIsUsingItem(&scene->players[0])) {
        scene->dropPenalty = mathfMoveTowards(scene->dropPenalty, 1.0f, FIXED_DELTA_TIME / TIME_TO_FILL_USE_BAR);

        if (mathfMod(scene->currentLevelTime, USE_ITEM_CHECK_INTERVAL) + FIXED_DELTA_TIME >= USE_ITEM_CHECK_INTERVAL) {
            sceneCheckSpawn(scene, &gZeroVec, MIN_ITEM_SPAWN_THRESHOLD);
        }
    } else if (scene->dropPenalty > 0.0f) {
        scene->dropPenalty -= FIXED_DELTA_TIME / TIME_TO_DECAY_FULL_BAR;

        if (scene->dropPenalty <= 0.0f) {
            scene->dropPenalty = 0.0f;
            bezosDeactivate(&scene->bezos);
        }
    }

    itemCoordinatorUpdate(&scene->itemCoordinator);

    collisionSceneCollide(&gCollisionScene);
    
    scene->currentLevelTime += FIXED_DELTA_TIME;

    scene->penaltyTime = mathfMoveTowards(scene->penaltyTime, 0.0f, FIXED_DELTA_TIME);
    scene->successTime = mathfMoveTowards(scene->successTime, 0.0f, FIXED_DELTA_TIME);
}

struct Colorf32 gAmbientLight = {0.0f, 0.1f, 0.2f, 1.0f};
struct Colorf32 gAmbientScale = {0.5f, 0.5f, 0.5f, 1.0f};
struct Colorf32 gLightColor = {0.3f, 0.3f, 0.15f, 1.0f};
struct Colorf32 gGreenLightColor = {0.3f, 0.5f, 0.3f, 1.0f};
struct Colorf32 gRedLightColor = {1.5f, 0.1f, 0.1f, 1.0f};
struct Colorf32 gOrangeLightColor = {1.0f, 0.2f, 0.1f, 1.0f};

struct Plane gGroundPlane = {{0.0f, 1.0f, 0.0}, -0.05f};

void sceneRender(struct Scene* scene, struct RenderState* renderState, struct GraphicsTask* task) {
    uiInitSpirtes(renderState);
    itemMarkNeedsRender();

    Mtx* identity = renderStateRequestMatrices(renderState, 1);
    guMtxIdent(identity);
    gSPMatrix(renderState->dl++, identity, G_MTX_LOAD | G_MTX_NOPUSH | G_MTX_MODELVIEW);

    for (int i = 0; i < scene->itemRequesterCount; ++i) {
        itemRequesterRenderGenerate(&scene->itemRequesters[i], i, renderState);
    }

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
    
    float viewPerspMatrix[4][4];

    cameraSetupMatrices(&scene->camera, renderState, (float)SCREEN_WD / (float)SCREEN_HT, viewPerspMatrix);
    
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
    shadowVolumeGroupInit(&shadowGroup, &scene->camera.transform, &viewPerspMatrix);
    
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
    
    for (int i = 0; i < scene->returnBinCount; ++i) {
        returnBinRender(&scene->returnBins[i], renderScene);
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
        itemRequesterRender(&scene->itemRequesters[i], i, renderScene);
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

    shadowVolumeGroupRender(&shadowGroup, renderState);

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

    if (bezosIsActive(&scene->bezos)) {
        effects |= PalleteEffectsGrayscaleRed;
    }

    float timeSinceBezos = gTimePassed - scene->appearTime;

    if (timeSinceBezos > 0.0f && timeSinceBezos < APPEAR_FLASH_PERIOD * APPEAR_FLASH_COUNT) {
        if (mathfMod(timeSinceBezos, APPEAR_FLASH_PERIOD) < APPEAR_FLASH_PERIOD * 0.5f) {
            effects |= PalleteEffectsInvert;
        }

        effects &= ~PalleteEffectsGrayscaleRed;
    }

    struct Colorf32* lightColor = &gLightColor;

    if (scene->successTime) {
        lightColor = &gGreenLightColor;
    } else if (mathfMod(scene->penaltyTime, RED_FLASH_TIME * 0.5f) > RED_FLASH_TIME * 0.25f) {
        lightColor = &gRedLightColor;
    } else if (playerIsUsingItem(&scene->players[0])) {
        lightColor = &gOrangeLightColor;
    }

    u16* pallete = palleteGenerateLit(
        (struct Coloru8*)ADJUST_POINTER_FOR_SEGMENT(pallete_half_pallete_rgba_32b, gMaterialSegment, MATERIAL_SEGMENT), 
        &gAmbientLight,
        &gAmbientScale, 
        lightColor, 
        effects,
        scene->fadeInTime ? 1.0f - scene->fadeInTime * (1.0f / FADE_IN_DURATION) : endScreenFadeAmount(&scene->endScreen),
        renderState
    );

    gDPLoadTLUT_pal256(renderState->dl++, pallete);

    if (bezosIsActive(&scene->bezos) || (playerIsUsingItem(&scene->players[0]) && mathfMod(scene->currentLevelTime, USE_ITEM_CHECK_INTERVAL) < 1.5f)) {
        gSPDisplayList(renderState->dl++, gCopyCBScaryMaterial);
    } else {
        gSPDisplayList(renderState->dl++, gCopyCBMaterial);
    }
    gSPDisplayList(renderState->dl++, gCopyCB);

    gDPPipeSync(renderState->dl++);
    gSPDisplayList(renderState->dl++, ui_default_ui);

    spriteInit(renderState);

    if (scene->itemCoordinator.totalItems && scene->fadeInTime == 0.0f && scene->endScreen.success == EndScreenTypeNone) {
        int xSlide = 9 + 40 * scene->itemCoordinator.itemsFulfulled / scene->itemCoordinator.totalItems;
        spriteDraw(renderState, ARROW_FILLED_INDEX, 533, 50, xSlide, 32, 0, 0, 0, 0);
        spriteDraw(renderState, ARROW_EMPTY_INDEX, 533 + xSlide, 50, 64 - xSlide, 32, xSlide, 0, 0, 0);
    }

    if (scene->endScreen.success == EndScreenTypeNone && pauseMenuIsPaused(&scene->pauseMenu)) {
        pauseMenuRender(&scene->pauseMenu, renderState);
    } else {
        tutorialRender(&scene->tutorial, renderState);
        endScreenRender(&scene->endScreen, renderState);
    }

    spriteFinish(renderState);

    // shadowMapRenderDebug(renderState, scene->players[0].shadowMap.buffer);
}

struct Item* scenePickupItem(struct Scene* scene, struct Vector3* grabFrom) {
    float spawnDelay = itemCoordinatorSpawnDelay(&scene->itemCoordinator);

    for (int i = 0; i < scene->conveyorCount; ++i) {
        struct Item* result = conveyorPickupItem(&scene->conveyors[i], grabFrom, spawnDelay);

        if (result) {
            soundPlayerPlay(SOUNDS_PICKITEMUP, 1.0f, 1.0f, NULL);
            return result;
        }
    }

    for (int i = 0; i < scene->tableCount; ++i) {
        struct Item* result = tablePickupItem(&scene->tables[i], grabFrom);

        if (result) {
            soundPlayerPlay(SOUNDS_PICKITEMUP, 1.0f, 1.0f, NULL);
            return result;
        }
    }
    

    return NULL;
}

int sceneDropItem(struct Scene* scene, struct Item* item, struct Vector3* dropAt) {
    int throwSuccess = 0;

    for (int i = 0; i < scene->itemRequesterCount; ++i) {
        enum ItemDropResult dropResult = itemRequesterDrop(&scene->itemRequesters[i], item, dropAt);
        if (dropResult) {
            if (dropResult == ItemDropResultSuccess) {
                soundPlayerPlay(SOUNDS_SUCCESFULREQUEST, 1.0f, 1.0f, NULL);
            
                itemCoordinatorMarkSuccess(&scene->itemCoordinator);

                if (itemCoordinatorDidWin(&scene->itemCoordinator)) {
                    endScreenEndGame(&scene->endScreen, gCurrentLevelIndex + 1 == levelGetCount() ? EndScreenTypeSuccessLastLevel : EndScreenTypeSuccess);
                    saveFileMarkLevelComplete(gCurrentLevelIndex, scene->currentLevelTime);
                }

                if (item->flags & ITEM_FLAGS_THROWN) {
                    throwSuccess = 1;
                }
            }

            scene->itemRequesters[i].requestDelay = itemCoordinatorPreDelay(&scene->itemCoordinator);

            return dropResult == ItemDropResultSuccess;
        }
    }

    for (int i = 0; i < scene->tableCount; ++i) {
        if (tableDropItem(&scene->tables[i], item, dropAt)) {
            soundPlayerPlay(SOUNDS_SETITEMDOWN, 0.7f, 1.0f, NULL);
            return 1;
        }
    }


    for (int i = 0; i < scene->returnBinCount; ++i) {
        if (returnBinDropItem(&scene->returnBins[i], item, dropAt)) {
            soundPlayerPlay(SOUNDS_TRASHITEM, 1.0f, 1.0f, NULL);
            return 1;
        }
    }

    // get a bonus for a successful throw
    if (throwSuccess) {
        for (int i = 0; i < scene->conveyorCount; ++i) {
            scene->conveyors[i].spawnDelay = 0.0f;
        }
    }

    return 0;
}

int sceneSwapItem(struct Scene* scene, struct Item* item, struct Vector3* dropAt, struct Item** replacement) {
    for (int i = 0; i < scene->tableCount; ++i) {
        if (tableSwapItem(&scene->tables[i], item, dropAt, replacement)) {
            soundPlayerPlay(SOUNDS_PICKITEMUP, 1.0f, 1.0f, NULL);
            soundPlayerPlay(SOUNDS_DROPITEM, 1.0f, 1.0f, NULL);
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


    for (int i = 0; i < scene->returnBinCount; ++i) {
        if (returnBinHover(&scene->returnBins[i], dropAt, hoverOutput)) {
            return 1;
        }
    }

    return 0;
}


struct Vector3* sceneNearestPlayerPos(struct Scene* scene) {
    return &scene->players[0].transform.position;
}
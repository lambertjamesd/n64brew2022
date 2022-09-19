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
}

unsigned ignoreInputFrames = 10;

void sceneUpdate(struct Scene* scene) {
    if (ignoreInputFrames) {
        --ignoreInputFrames;
    }

    for (int i = 0; i < scene->playerCount; ++i) {
        playerUpdate(&scene->players[i]);
    }

    for (int i = 0; i < scene->spotLightCount; ++i) {
        spotLightUpdate(&scene->spotLights[i], &scene->camera.transform.position);
    }
}

struct Colorf32 gAmbientLight = {0.0f, 0.2f, 0.4f, 255};
struct Colorf32 gAmbientScale = {0.5f, 0.5f, 0.5f, 255};
struct Colorf32 gLightColor = {0.3f, 0.3f, 0.15f, 255};

struct Plane gGroundPlane = {{0.0f, 1.0f, 0.0}, -0.05f};

void sceneRender(struct Scene* scene, struct RenderState* renderState, struct GraphicsTask* task) {
    struct LightConfiguration playerLightConfig[scene->playerCount];

    for (int i = 0; i < scene->playerCount; ++i) {
        playerSetupTransforms(&scene->players[i], renderState);
    }

    for (int i = 0; i < scene->playerCount; ++i) {
        spotLightsFindConfiguration(scene->spotLights, scene->spotLightCount, &scene->players[i].transform.position, NULL, &playerLightConfig[i]);

        struct Vector3 lightPosition;

        if (spotLightsGetPosition(&playerLightConfig[i], &lightPosition)) {  
            Gfx* playerShadowGfx = playerGenerateShadowMapGfx(&scene->players[i], renderState);

            shadowMapRender(
                &scene->players[i].shadowMap, 
                renderState, 
                task, 
                &lightPosition, 
                &scene->players[i].transform, 
                playerShadowGfx
            );

            scene->players[i].shadowMap.flags |= SHADOW_MAP_ENABLED;
        } else {
            scene->players[i].shadowMap.flags &= ~SHADOW_MAP_ENABLED;
        }
    }

    Mtx* identity = renderStateRequestMatrices(renderState, 1);
    guMtxIdent(identity);
    gSPMatrix(renderState->dl++, identity, G_MTX_LOAD | G_MTX_NOPUSH | G_MTX_MODELVIEW);

    gDPPipeSync(renderState->dl++);
    gDPSetColorImage(renderState->dl++, G_IM_FMT_CI, G_IM_SIZ_8b, SCREEN_WD, indexColorBuffer);
    gDPSetCycleType(renderState->dl++, G_CYC_FILL);
    gDPSetFillColor(renderState->dl++, 0x02020202);
    gDPFillRectangle(renderState->dl++, 0, 0, SCREEN_WD-1, SCREEN_HT-1);
    gDPPipeSync(renderState->dl++);
    gDPSetCycleType(renderState->dl++, G_CYC_1CYCLE);

    struct FrustrumCullingInformation cullingInformation;

    cameraSetupMatrices(&scene->camera, renderState, (float)SCREEN_WD / (float)SCREEN_HT, &fullscreenViewport, &cullingInformation);
    
    gSPSetLights1(renderState->dl++, gLights);
    gDPSetRenderMode(renderState->dl++, G_RM_OPA_SURF, G_RM_OPA_SURF2);
    gSPClearGeometryMode(renderState->dl++, G_ZBUFFER);

    struct RenderScene* renderScene = renderSceneNew(&scene->camera.transform, renderState, RENDER_SCENE_CAPACITY, ~0, 0);

    for (unsigned i = 0; i < gCurrentLevel->groundContentCount; ++i) {
        renderSceneAdd(renderScene, gCurrentLevel->groundContent[i].displayList, NULL, gCurrentLevel->groundContent[i].materialIndex, &gZeroVec, NULL);
    }

    renderSceneGenerate(renderScene, renderState);
    renderSceneFree(renderScene);

    gDPPipeSync(renderState->dl++);
    gDPSetRenderMode(renderState->dl++, G_RM_ZB_OPA_SURF, G_RM_ZB_OPA_SURF2);
    gSPSetGeometryMode(renderState->dl++, G_ZBUFFER);
    
    
    renderScene = renderSceneNew(&scene->camera.transform, renderState, RENDER_SCENE_CAPACITY, ~0, levelMaterialDefault());

    for (unsigned i = 0; i < gCurrentLevel->staticContentCount; ++i) {
        renderSceneAdd(renderScene, gCurrentLevel->staticContent[i].displayList, NULL, gCurrentLevel->staticContent[i].materialIndex, &gZeroVec, NULL);
    }

    for (unsigned i = 0; i < scene->playerCount; ++i) {
        spotLightsSetupLight(&playerLightConfig[i], &scene->players[i].transform.position, renderState);
        
        playerRender(&scene->players[i], renderScene);
    }

    renderSceneGenerate(renderScene, renderState);
    renderSceneFree(renderScene);

    for (unsigned i = 0; i < scene->playerCount; ++i) {
        shadowMapRenderOntoPlane(&scene->players[i].shadowMap, renderState, &gGroundPlane);
    }

    gDPPipeSync(renderState->dl++);
    gDPSetRenderMode(renderState->dl++, G_RM_ZB_OPA_SURF, G_RM_ZB_OPA_SURF2);
    gSPSetGeometryMode(renderState->dl++, G_ZBUFFER);
    gDPSetAlphaCompare(renderState->dl++, G_AC_NONE);
    gDPSetBlendColor(renderState->dl++, 0, 0, 0, 0);
    gSPDisplayList(renderState->dl++, levelMaterial(ADDITIVE_LIGHT_INDEX));

    for (unsigned i = 0; i < scene->spotLightCount; ++i) {
        spotLightRenderProjection(&scene->spotLights[i], renderState);
    }

    gSPGeometryMode(renderState->dl++, G_CULL_FRONT, G_CULL_BACK);
    gDPSetRenderMode(renderState->dl++, G_RM_ZB_OPA_SURF, G_RM_ZB_OPA_SURF2);

    struct Transform transform;
    transformInitIdentity(&transform);

    gDPPipeSync(renderState->dl++);
    gDPSetCycleType(renderState->dl++, G_CYC_1CYCLE);
    gDPSetColorImage(renderState->dl++, G_IM_FMT_RGBA, G_IM_SIZ_16b, SCREEN_WD, osVirtualToPhysical(task->framebuffer));
    gSPSegment(renderState->dl++, SOURCE_CB_SEGMENT, indexColorBuffer);

    u16* pallete = palleteGenerateLit((struct Coloru8*)pallete_half_pallete_rgba_32b, &gAmbientLight, &gAmbientScale, &gLightColor, renderState);

    gDPLoadTLUT_pal256(renderState->dl++, pallete);

    gSPDisplayList(renderState->dl++, gCopyCB);

    gDPSetRenderMode(renderState->dl++, G_RM_ZB_OPA_SURF, G_RM_ZB_OPA_SURF2);

    shadowMapRenderDebug(renderState, scene->players[0].shadowMap.buffer);
}
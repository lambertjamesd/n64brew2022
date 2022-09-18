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

#define OBJECT_COUNT    3

struct Transform gObjectPos[OBJECT_COUNT] = {
    {{-1.5f, 0.0f, -2.5f}, {0.707f, 0.0f, 0.0f, 0.707f}, {1.0f, 1.0f, 1.0f}},
    {{2.5f, 0.0f, -2.0f}, {0.707f, 0.0f, 0.0f, 0.707f}, {1.0f, 1.0f, 1.0f}},
    {{0.0f, 0.0f * 0.75f, 1.5f}, {-0.923879533f, 0.0f, 0.0f, 0.382683432f}, {1.0f, 1.0f, 1.0f}},
};

Gfx* gObjectGfx[OBJECT_COUNT] = {
    sphere_model_gfx,
    cylinder_model_gfx,
    suzanne_model_gfx,
};

struct Vector3 gLightOrbitCenter = {0.0f, 5.0f, 0.0f};

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

    pointLightInit(&scene->pointLight, &gLightOrbitCenter, &gColorWhite, 15.0f);

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
    float angle = gTimePassed * 2.0f * M_PI / LIGHT_ORBIT_PERIOD;

    scene->pointLight.position.x = cosf(angle) * LIGHT_ORBIT_RADIUS + gLightOrbitCenter.x;
    scene->pointLight.position.y = cosf(angle * 3.0f) + gLightOrbitCenter.y;
    scene->pointLight.position.z = sinf(angle) * LIGHT_ORBIT_RADIUS + gLightOrbitCenter.z;

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

void sceneRenderObject(struct Scene* scene, struct RenderState* renderState, Gfx* model, struct Transform* transform, int objectIndex) {
    Mtx* mtxTransform = renderStateRequestMatrices(renderState, 1);

    Light* light = renderStateRequestLights(renderState, 1);

    pointLightCalculateLightDirOnly(&scene->pointLight, &transform->position, light);

    gSPLight(renderState->dl++, light, 1);

    transformToMatrixL(transform, mtxTransform, SCENE_SCALE);
    materialSetToon(renderState, objectIndex);
    gSPMatrix(renderState->dl++, mtxTransform, G_MTX_MODELVIEW | G_MTX_PUSH | G_MTX_MUL);
    gSPDisplayList(renderState->dl++, model);

    gSPPopMatrix(renderState->dl++, G_MTX_MODELVIEW);
}

struct Colorf32 gAmbientLight = {0.0f, 0.2f, 0.4f, 255};
struct Colorf32 gAmbientScale = {0.5f, 0.5f, 0.5f, 255};
struct Colorf32 gLightColor = {0.3f, 0.3f, 0.15f, 255};

struct Plane gGroundPlane = {{0.0f, 1.0f, 0.0}, -0.1f};

void sceneRender(struct Scene* scene, struct RenderState* renderState, struct GraphicsTask* task) {
    struct LightConfiguration playerLightConfig[scene->playerCount];

    // struct Plane groundPlane;
    // groundPlane.normal = gUp;
    // groundPlane.d = 0.0f;

    for (int i = 0; i < scene->playerCount; ++i) {
        spotLightsFindConfiguration(scene->spotLights, scene->spotLightCount, &scene->players[i].transform.position, NULL, &playerLightConfig[i]);

        struct Vector3 lightPosition;

        if (spotLightsGetPosition(&playerLightConfig[i], &lightPosition)) {  
            shadowMapRender(
                &scene->players[i].shadowMap, 
                renderState, 
                task, 
                &lightPosition, 
                &scene->players[i].transform, 
                suzanne_model_gfx
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

    for (unsigned i = 0; i < OBJECT_COUNT; ++i) {
        sceneRenderObject(
            scene, 
            renderState, 
            gObjectGfx[i], 
            &gObjectPos[i], 
            i
        );
    }

    gSPGeometryMode(renderState->dl++, G_CULL_FRONT, G_CULL_BACK);
    gDPSetRenderMode(renderState->dl++, G_RM_ZB_OPA_SURF, G_RM_ZB_OPA_SURF2);

    struct Transform transform;
    transformInitIdentity(&transform);

    gSPDisplayList(renderState->dl++, point_light_mat);

    Mtx* lightMtx = renderStateRequestMatrices(renderState, 1);
    transform.position = scene->pointLight.position;
    vector3Scale(&gOneVec, &transform.scale, 0.25f);
    transformToMatrixL(&transform, lightMtx, SCENE_SCALE);

    gDPSetEnvColor(renderState->dl++, 255, 255, 255, 255);
    gDPSetCombineLERP(renderState->dl++, 0, 0, 0, ENVIRONMENT, 0, 0, 0, ENVIRONMENT, 0, 0, 0, ENVIRONMENT, 0, 0, 0, ENVIRONMENT);
    gSPMatrix(renderState->dl++, lightMtx, G_MTX_MODELVIEW | G_MTX_PUSH | G_MTX_MUL);
    gSPDisplayList(renderState->dl++, sphere_model_gfx);
    gSPPopMatrix(renderState->dl++, G_MTX_MODELVIEW);

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
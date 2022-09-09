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

#include "../build/assets/materials/static.h"
#include "../build/assets/materials/pallete.h"

#define ROTATE_PER_SECOND       (M_PI * 0.25f)
#define MOVE_PER_SECOND         (3.0f)
#define MIN_DISTANCE            (2.0f)
#define MAX_DISTANCE            (20.0f)

#define RENDER_SCENE_CAPACITY   256

Lights1 gLights = gdSPDefLights1(0x10, 0, 0, 0xE0, 0xE0, 0xE0, 90, 90, 0);

struct Vector3 gCameraFocus = {0.0f, 0.0f, 0.0f};
struct Vector3 gCameraStart = {0.0f, 2.0f, 5.0f};
float gCameraDistance = 0.0f;

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

void sceneInit(struct Scene* scene) {
    cameraInit(&scene->camera, 90.0f, 0.5f * SCENE_SCALE, 30.0f * SCENE_SCALE);

    gCameraDistance = sqrtf(vector3DistSqrd(&gCameraFocus, &gCameraStart));

    scene->camera.transform.position = gCameraStart;
    struct Vector3 offset;
    vector3Sub(&gCameraFocus, &gCameraStart, &offset);
    quatLook(&offset, &gUp, &scene->camera.transform.rotation);

    pointLightInit(&scene->pointLight, &gLightOrbitCenter, &gColorWhite, 15.0f);
}

unsigned ignoreInputFrames = 10;

void sceneUpdate(struct Scene* scene) {
    OSContPad* input = controllersGetControllerData(0);

    struct Quaternion rotate;
    quatAxisAngle(&gUp, (ROTATE_PER_SECOND * (1.0f / 80.0f)) * FIXED_DELTA_TIME * input->stick_x, &rotate);
    struct Quaternion finalRotation;
    quatMultiply(&rotate, &scene->camera.transform.rotation, &finalRotation);
    scene->camera.transform.rotation = finalRotation;

    quatAxisAngle(&gRight, -(ROTATE_PER_SECOND * (1.0f / 80.0f)) * FIXED_DELTA_TIME * input->stick_y, &rotate);
    quatMultiply(&scene->camera.transform.rotation, &rotate, &finalRotation);
    scene->camera.transform.rotation = finalRotation;

    if (!ignoreInputFrames && controllerGetButton(0, A_BUTTON)) {
        gCameraDistance -= MOVE_PER_SECOND * FIXED_DELTA_TIME;
    }

    if (!ignoreInputFrames && controllerGetButton(0, B_BUTTON)) {
        gCameraDistance += MOVE_PER_SECOND * FIXED_DELTA_TIME;
    }

    gCameraDistance = MAX(MIN_DISTANCE, gCameraDistance);
    gCameraDistance = MIN(MAX_DISTANCE, gCameraDistance);

    struct Vector3 offset;
    quatMultVector(&scene->camera.transform.rotation, &gForward, &offset);
    vector3AddScaled(&gCameraFocus, &offset, gCameraDistance, &scene->camera.transform.position);

    float angle = gTimePassed * 2.0f * M_PI / LIGHT_ORBIT_PERIOD;

    scene->pointLight.position.x = cosf(angle) * LIGHT_ORBIT_RADIUS + gLightOrbitCenter.x;
    scene->pointLight.position.y = cosf(angle * 3.0f) + gLightOrbitCenter.y;
    scene->pointLight.position.z = sinf(angle) * LIGHT_ORBIT_RADIUS + gLightOrbitCenter.z;

    if (ignoreInputFrames) {
        --ignoreInputFrames;
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

struct Coloru8 gShadowColor = {100, 100, 100, 255};
struct Coloru8 gLightColor = {255, 255, 255, 255};

void sceneRender(struct Scene* scene, struct RenderState* renderState, struct GraphicsTask* task) {
    struct RenderScene* renderScene = renderSceneNew(&scene->camera.transform, renderState, RENDER_SCENE_CAPACITY, ~0);

    gDPPipeSync(renderState->dl++);
    gDPSetColorImage(renderState->dl++, G_IM_FMT_CI, G_IM_SIZ_8b, SCREEN_WD, indexColorBuffer);
    gDPSetCycleType(renderState->dl++, G_CYC_FILL);
    gDPSetFillColor(renderState->dl++, 0x02020202);
    gDPFillRectangle(renderState->dl++, 0, 0, SCREEN_WD-1, SCREEN_HT-1);
    gDPPipeSync(renderState->dl++);
    gDPSetCycleType(renderState->dl++, G_CYC_1CYCLE);
    gDPSetRenderMode(renderState->dl++, G_RM_ZB_OPA_SURF, G_RM_ZB_OPA_SURF2);

    struct FrustrumCullingInformation cullingInformation;

    cameraSetupMatrices(&scene->camera, renderState, (float)SCREEN_WD / (float)SCREEN_HT, &fullscreenViewport, &cullingInformation);
    
    gSPSetLights1(renderState->dl++, gLights);
    
    for (unsigned i = 0; i < gCurrentLevel->staticContentCount; ++i) {
        renderSceneAdd(renderScene, gCurrentLevel->staticContent[i].displayList, NULL, gCurrentLevel->staticContent[i].materialIndex, &gZeroVec, NULL);
    }

    renderSceneGenerate(renderScene, renderState);

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

    u16* pallete = palleteGenerateLit((struct Coloru8*)pallete_half_pallete_rgba_32b, gShadowColor, gLightColor, renderState);

    gDPLoadTLUT_pal256(renderState->dl++, pallete);

    gSPDisplayList(renderState->dl++, gCopyCB);

    gDPSetRenderMode(renderState->dl++, G_RM_ZB_OPA_SURF, G_RM_ZB_OPA_SURF2);

    renderSceneFree(renderScene);
}
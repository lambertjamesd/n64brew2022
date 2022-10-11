#include "bezos.h"

#include "../build/assets/models/ghostjeff.h"
#include "../build/assets/models/pumpkin.h"
#include "../build/assets/materials/static.h"

#include "../defs.h"

void bezosInit(struct Bezos* bezos) {
    transformInitIdentity(&bezos->transform);

    skAnimatorInit(&bezos->animator, GHOSTJEFF_DEFAULT_BONES_COUNT, NULL, NULL);
    skArmatureInit(&bezos->armature, ghostjeff_model_gfx, GHOSTJEFF_DEFAULT_BONES_COUNT, ghostjeff_default_bones, ghostjeff_bone_parent, GHOSTJEFF_ATTACHMENT_COUNT);

    bezos->flags = 0;
}

void bezosActivate(struct Bezos* bezos, struct Vector3* at) {
    transformInitIdentity(&bezos->transform);
    bezos->transform.position = *at;
    bezos->transform.position.y = 0.0f;

    skAnimatorRunClip(&bezos->animator, &ghostjeff_animations[GHOSTJEFF_GHOSTJEFF_JEFFIDLE_INDEX], SKAnimatorFlagsLoop);

    bezos->flags |= BezosFlagsActive;
}

void bezosDeactivate(struct Bezos* bezos) {
    skAnimatorRunClip(&bezos->animator, &ghostjeff_animations[GHOSTJEFF_GHOSTJEFF_JEFFIDLE_INDEX], 0);

    bezos->flags &= ~BezosFlagsActive;
}

void bezosUpdate(struct Bezos* bezos) {
    skAnimatorUpdate(&bezos->animator, bezos->armature.boneTransforms, 1.0f);
}

void bezosRender(struct Bezos* bezos, struct SpotLight* spotLights, int spotLightCount, struct RenderScene* renderScene) {
    if (!(bezos->flags & BezosFlagsActive)) {
        return;
    }

    Mtx* matrix = renderStateRequestMatrices(renderScene->renderState, 1);

    transformToMatrixL(&bezos->transform, matrix, SCENE_SCALE);

    Mtx* armature = renderStateRequestMatrices(renderScene->renderState, GHOSTJEFF_DEFAULT_BONES_COUNT);
    skCalculateTransforms(&bezos->armature, armature);

    struct LightConfiguration lightConfig;

    spotLightsFindConfiguration(spotLights, spotLightCount, &bezos->transform.position, 0.0f, &lightConfig);

    Light* light = spotLightsSetupLight(&lightConfig, &bezos->transform.position, renderScene->renderState);

    renderSceneAdd(renderScene, ghostjeff_model_gfx, matrix, PLAYER_0_INDEX, &bezos->transform.position, armature, light);
}
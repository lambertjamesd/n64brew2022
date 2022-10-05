#include "item_requester.h"

#include "../build/assets/models/ui/item_prompt.h"
#include "../build/assets/materials/static.h"

#include "../defs.h"

#include "../util/time.h"

#include "../math/mathf.h"

#include "item_render.h"

#define OSCILATE_PERIOD     2.0f
#define OSCILATE_HEIGHT     0.25f

void itemRequesterInit(struct ItemRequester* requester, struct ItemRequesterDefinition* definition) {
    requester->transform.position = definition->position;
    requester->transform.rotation = definition->rotation;
    requester->transform.scale = gOneVec;

    requester->requestedType = ItemTypeCount;
    requester->timeLeft = 0.0f;
}

void itemRequesterUpdate(struct ItemRequester* requester) {
    if (requester->timeLeft > 0.0f) {
        requester->timeLeft -= FIXED_DELTA_TIME;

        if (requester->timeLeft <= 0.0f) {
            requester->timeLeft = 0.0f;
            requester->requestedType = ItemTypeCount;
        }
    }
}

void itemRequesterRender(struct ItemRequester* requester, struct RenderScene* renderScene) {
    Mtx* matrix = renderStateRequestMatrices(renderScene->renderState, 1);

    struct Transform signTransform;
    signTransform.position = requester->transform.position;
    signTransform.position.y += OSCILATE_HEIGHT * sinf(gTimePassed * (M_PI * 2.0f / OSCILATE_PERIOD));
    signTransform.rotation = renderScene->cameraTransform.rotation;
    signTransform.scale = gOneVec;
    transformToMatrixL(&signTransform, matrix, SCENE_SCALE);

    Gfx* gfx = itemRenderUseImage(ItemTypePumpkin, renderScene->renderState, ui_item_prompt_model_gfx);

    renderSceneAdd(renderScene, gfx, matrix, ITEM_PROMPT_INDEX, &requester->transform.position, NULL, NULL);
}
#include "item_render.h"

#include <ultra64.h>

#include "../scene/camera.h"
#include "../level/level.h"

#include "../sk64/skelatool_defs.h"

#include "../graphics/graphics.h"
#include "../util/time.h"

#include "./item.h"

static Vp gItemRenderViewport = {
  .vp = {
    .vscale = {ITEM_RENDER_SIZE*2, ITEM_RENDER_SIZE*2, G_MAXZ/2, 0},	/* scale */
    .vtrans = {ITEM_RENDER_SIZE*2, ITEM_RENDER_SIZE*2, G_MAXZ/2, 0},	/* translate */
  }
};

Lights1 gRenderLights = gdSPDefLights1(0, 0, 0, 255, 255, 255, 127, 0, 0);

u8 __attribute__((aligned(64))) gItemImages[MAX_ITEM_REQUESTERS][ITEM_RENDER_SIZE * ITEM_RENDER_SIZE];

int gItemImagesRendered = 0;

extern u8 __attribute__((aligned(64))) indexColorBuffer[SCREEN_HT * SCREEN_WD];

struct ColorThreshold {
    int colorIndex;
    float durationThreshold;
};

struct ColorThreshold thresholds[] = {
    {0x39393939, 12.0f},
    {0x01010101, 6.0f},
    {0x41414141, 0.0f},
};

void itemRenderGenerate(int itemIndex, enum ItemType itemType, float progress, float timeLeft, struct RenderState* renderState) {
    struct ItemTypeDefinition* itemDef = &gItemDefinitions[itemType];

    gDPSetColorImage(renderState->dl++, G_IM_FMT_I, G_IM_SIZ_8b, ITEM_RENDER_SIZE, gItemImages[itemIndex]);

    int progressX = (int)(ITEM_RENDER_SIZE * progress);
    
    gDPSetCycleType(renderState->dl++, G_CYC_FILL);
    gDPSetFillColor(renderState->dl++, 0x25252525);

    gDPFillRectangle(renderState->dl++, 0, 0, progressX-1, ITEM_RENDER_SIZE-1);
    gDPPipeSync(renderState->dl++);

    for (int i = 0; i < sizeof(thresholds) / sizeof(*thresholds); ++i) {
        if (thresholds[i].durationThreshold * DIFFICULTY_SCALAR <= timeLeft) {
            gDPSetFillColor(renderState->dl++, thresholds[i].colorIndex);
            break;
        }
    }

    gDPFillRectangle(renderState->dl++, progressX, 0, ITEM_RENDER_SIZE-1, ITEM_RENDER_SIZE-1);
    gSPViewport(renderState->dl++, &gItemRenderViewport);
    gDPSetScissor(renderState->dl++, G_SC_NON_INTERLACE, 0, 0, ITEM_RENDER_SIZE, ITEM_RENDER_SIZE);

    // setup camera
    if (itemDef->cameraDefinition) {
        struct Camera camera;
        cameraInit(
            &camera, 
            itemDef->cameraDefinition->verticalFov, 
            itemDef->cameraDefinition->nearPlane * SCENE_SCALE, 
            itemDef->cameraDefinition->farPlane * SCENE_SCALE
        );
        camera.transform.position = itemDef->cameraDefinition->position;
        camera.transform.rotation = itemDef->cameraDefinition->rotation;

        float viewPerspMatrix[4][4];
        cameraSetupMatrices(&camera, renderState, 1.0f, viewPerspMatrix);
    } else {
        Mtx* projection = renderStateRequestMatrices(renderState, 1);
        guOrtho(projection, 
            -0.5f * SCENE_SCALE, 0.5f * SCENE_SCALE, 
            -0.5f * SCENE_SCALE, 0.5f * SCENE_SCALE, 
            -1.0f * SCENE_SCALE, 1.0f * SCENE_SCALE,
            1.0f
        );
        gSPMatrix(renderState->dl++, projection, G_MTX_LOAD | G_MTX_PROJECTION | G_MTX_NOPUSH);
    }
    
    // set item material
    gDPPipeSync(renderState->dl++);
    gSPSetLights1(renderState->dl++, gRenderLights);

    if (itemDef->lightDir) {
        Light* light = renderStateRequestLights(renderState, 1);

        light->l.col[0] = 255;
        light->l.col[1] = 255;
        light->l.col[2] = 255;

        light->l.colc[0] = 255;
        light->l.colc[1] = 255;
        light->l.colc[2] = 255;

        vector3ToVector3u8(itemDef->lightDir, (struct Vector3u8*)light->l.dir);

        gSPLight(renderState->dl++, light, 1);
    }

    gSPDisplayList(renderState->dl++, levelMaterialDefault());
    gSPDisplayList(renderState->dl++, levelMaterial(itemDef->materialIndex));
    gDPSetRenderMode(renderState->dl++, G_RM_OPA_SURF, G_RM_OPA_SURF2);
    gSPGeometryMode(renderState->dl++, G_ZBUFFER, G_LIGHTING);

    // setup item armature
    if (itemDef->boneCount) {
        gSPSegment(renderState->dl++, MATRIX_TRANSFORM_SEGMENT, itemGetIdle(itemType, renderState));
    }

    // render item
    gSPDisplayList(renderState->dl++, itemDef->dl);
}

Gfx* itemRenderUseImage(int itemIndex, struct RenderState* renderState, Gfx* promptGfx) {
    Gfx* result = renderStateAllocateDLChunk(renderState, 10);

    Gfx* dl = result;

    gDPLoadTextureBlock(
        dl++, 
        gItemImages[itemIndex], 
        G_IM_FMT_I, G_IM_SIZ_8b, 
        ITEM_RENDER_SIZE, ITEM_RENDER_SIZE, 0, 
        G_TX_CLAMP | G_TX_NOMIRROR, G_TX_CLAMP | G_TX_NOMIRROR,
        8, 8,
        G_TX_NOLOD, G_TX_NOLOD
    );

    gSPDisplayList(dl++, promptGfx);
    gSPEndDisplayList(dl++);

    return result;
}
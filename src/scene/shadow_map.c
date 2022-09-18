#include "shadow_map.h"
#include "util/memory.h"
#include "math/mathf.h"
#include "math/matrix.h"
#include "graphics/graphics.h"
#include "math/plane.h"
#include "../defs.h"

static Vp shadowMapViewport = {
  .vp = {
    .vscale = {SHADOW_MAP_WIDTH*2, SHADOW_MAP_HEIGHT*2, G_MAXZ/2, 0},	/* scale */
    .vtrans = {SHADOW_MAP_WIDTH*2, SHADOW_MAP_HEIGHT*2, G_MAXZ/2, 0},	/* translate */
  }
};

#define SHADOW_MAP_COMBINE_MODE        0, 0, 0, ENVIRONMENT, 0, 0, 0, ENVIRONMENT

#define SHADOW_PROJECTION_COMBINE_MODE 0, 0, 0, ENVIRONMENT, 0, 0, 0, TEXEL0

#define	RM_UPDATE_Z(clk)		\
    Z_CMP | Z_UPD | IM_RD | CVG_DST_WRAP | CLR_ON_CVG |	\
	FORCE_BL | ZMODE_XLU |                          \
	GBL_c##clk(G_BL_CLR_IN, G_BL_A_IN, G_BL_CLR_MEM, G_BL_1MA)

Vtx gShadowMapVtxDataTemplate[] = {
    {{{100, 0, 100}, 0, {SHADOW_MAP_WIDTH << 5, 0}, {255, 255, 255, 255}}},
    {{{-100, 0, 100}, 0, {0, 0}, {255, 255, 255, 255}}},
    {{{-100, 0, -100}, 0, {0, SHADOW_MAP_HEIGHT << 5}, {255, 255, 255, 255}}},
    {{{100, 0, -100}, 0, {SHADOW_MAP_WIDTH << 5, SHADOW_MAP_HEIGHT << 5}, {255, 255, 255, 255}}},
};

struct Vector3 shadowCornerConfig[] = {
    {1.0f, 1.0f, 1.0f},
    {-1.0f, 1.0f, 1.0f},
    {-1.0f, -1.0f, 1.0f},
    {1.0f, -1.0f, 1.0f},
};

Gfx shadowMapMaterial[] = {
    gsDPPipeSync(),
    gsDPSetEnvColor(255, 255, 255, 255),
    gsDPSetRenderMode(G_RM_OPA_SURF, G_RM_OPA_SURF2),
    gsDPSetCombineMode(SHADOW_MAP_COMBINE_MODE, SHADOW_MAP_COMBINE_MODE),
    gsSPGeometryMode(G_LIGHTING | G_SHADE | G_ZBUFFER, 0),
    gsSPEndDisplayList(),
};

void shadowMapRenderOntoPlane(struct ShadowMap* shadowMap, struct RenderState* renderState, struct Plane* ontoPlane) {
    if (!(shadowMap->flags & SHADOW_MAP_ENABLED)) {
        return;
    }

    Vtx* vertices = renderStateRequestVertices(renderState, 4);
    Vtx* currVtx = vertices;

    for (unsigned i = 0; i < 4; ++i) {
        *currVtx = gShadowMapVtxDataTemplate[i];

        struct Vector3 localSpace;
        localSpace.x = shadowMap->projOffset;
        localSpace.y = shadowMap->projOffset;
        localSpace.z = -shadowMap->nearPlane;
        vector3Multiply(&localSpace, &shadowCornerConfig[i], &localSpace);
        transformPoint(&shadowMap->lightPovTransform, &localSpace, &localSpace);
        struct Vector3 rayDir;
        vector3Sub(&localSpace, &shadowMap->lightPovTransform.position, &rayDir);
        vector3Normalize(&rayDir, &rayDir);

        float rayDistance = 0.0f;

        if (!planeRayIntersection(ontoPlane, &shadowMap->lightPovTransform.position, &rayDir, &rayDistance)) {
            return;
        }

        struct Vector3 intersectPoint;
        vector3AddScaled(&shadowMap->lightPovTransform.position, &rayDir, rayDistance, &intersectPoint);

        currVtx->v.ob[0] = (short)(intersectPoint.x * SCENE_SCALE);
        currVtx->v.ob[1] = (short)(intersectPoint.y * SCENE_SCALE);
        currVtx->v.ob[2] = (short)(intersectPoint.z * SCENE_SCALE);

        ++currVtx;
    }

    gDPPipeSync(renderState->dl++);
    gDPSetCycleType(renderState->dl++, G_CYC_1CYCLE);
    gDPSetTextureLUT(renderState->dl++, G_TT_NONE);
    gDPSetRenderMode(renderState->dl++, RM_UPDATE_Z(1), RM_UPDATE_Z(2));
    gSPGeometryMode(renderState->dl++, G_LIGHTING | G_SHADE | G_ZBUFFER, 0);
    gDPSetAlphaCompare(renderState->dl++, G_AC_THRESHOLD);
    gDPSetCombineMode(renderState->dl++, SHADOW_PROJECTION_COMBINE_MODE, SHADOW_PROJECTION_COMBINE_MODE);
    gDPSetEnvColor(renderState->dl++, 0, 0, 0, 255);
    gDPSetBlendColor(renderState->dl++, 128, 128, 128, 128);
    gDPTileSync(renderState->dl++);
    gDPLoadTextureTile(
        renderState->dl++,
        shadowMap->buffer,
        G_IM_FMT_I, G_IM_SIZ_8b,
        SHADOW_MAP_WIDTH, SHADOW_MAP_HEIGHT,
        0, 0,
        SHADOW_MAP_WIDTH-1, SHADOW_MAP_HEIGHT-1,
        0,
        G_TX_CLAMP | G_TX_NOMIRROR, G_TX_CLAMP | G_TX_NOMIRROR,
        0, 0, 
        0, 0
    );

    Gfx* shadowMapGfx = renderStateAllocateDLChunk(renderState, 3);
    Gfx* dl = shadowMapGfx;

    gSPVertex(dl++, vertices, 4, 0);
    gSP2Triangles(dl++, 0, 1, 2, 0, 0, 2, 3, 0);
    gSPEndDisplayList(dl++);

    gSPDisplayList(renderState->dl++, shadowMapGfx);
}

void shadowMapRender(struct ShadowMap* shadowMap, struct RenderState* renderState, struct GraphicsTask* gfxTask, struct Vector3* from, struct Transform* subjectTransform, Gfx* subject) {
    struct Vector3 offset;
    vector3Sub(&subjectTransform->position, from, &offset);

    float distance = sqrtf(vector3MagSqrd(&offset));

    float subjectRadius = shadowMap->subjectRadius * subjectTransform->scale.x;

    float nearPlane = distance - subjectRadius;

    if (nearPlane < 0.00001f) {
        return;
    }
    
    float projOffset = SCENE_SCALE * nearPlane * subjectRadius / sqrtf(distance * distance - subjectRadius * subjectRadius);

    if (projOffset < 0.00001f) {
        return;
    }

    float projMatrix[4][4];
    u16 perspNorm;
    matrixPerspective(projMatrix, &perspNorm, -projOffset, projOffset, projOffset, -projOffset, nearPlane * SCENE_SCALE, (distance + subjectRadius) * SCENE_SCALE);

    shadowMap->lightPovTransform.position = *from;
    shadowMap->lightPovTransform.scale = gOneVec;
    quatLook(&offset, &gUp, &shadowMap->lightPovTransform.rotation);

    struct Transform povInverse;
    transformInvert(&shadowMap->lightPovTransform, &povInverse);
    float cameraView[4][4];
    transformToMatrix(&povInverse, cameraView, SCENE_SCALE);
    float viewProj[4][4];
    guMtxCatF(cameraView, projMatrix, viewProj);

    float subjectMatrix[4][4];
    transformToMatrix(subjectTransform, subjectMatrix, SCENE_SCALE);
    guMtxCatF(subjectMatrix, viewProj, projMatrix);

    Mtx* lightMtx = renderStateRequestMatrices(renderState, 1);
    Mtx* identity = renderStateRequestMatrices(renderState, 1);
    guMtxIdent(identity);

    guMtxF2L(projMatrix, lightMtx);
    gSPMatrix(renderState->dl++, lightMtx, G_MTX_PROJECTION | G_MTX_NOPUSH | G_MTX_LOAD);
    gSPMatrix(renderState->dl++, identity, G_MTX_MODELVIEW | G_MTX_NOPUSH | G_MTX_LOAD);
    gDPPipeSync(renderState->dl++);
    gDPSetCycleType(renderState->dl++, G_CYC_FILL);
    gDPSetColorImage(renderState->dl++, G_IM_FMT_CI, G_IM_SIZ_8b, SHADOW_MAP_WIDTH, osVirtualToPhysical(shadowMap->buffer));
    gDPSetScissor(renderState->dl++, G_SC_NON_INTERLACE, 0, 0, SHADOW_MAP_WIDTH, SHADOW_MAP_HEIGHT);
    gSPViewport(renderState->dl++, &shadowMapViewport);
    gDPSetFillColor(renderState->dl++, 0);
    gDPFillRectangle(renderState->dl++, 0, 0, SHADOW_MAP_WIDTH-1, SHADOW_MAP_HEIGHT-1);
    gDPPipeSync(renderState->dl++);
    gDPSetCycleType(renderState->dl++, G_CYC_1CYCLE);

    gSPDisplayList(renderState->dl++, shadowMapMaterial);
    gSPDisplayList(renderState->dl++, subject);

    gDPPipeSync(renderState->dl++);
    gDPSetScissor(renderState->dl++, G_SC_NON_INTERLACE, 0, 0, SCREEN_WD, SCREEN_HT);
    gSPViewport(renderState->dl++, &fullscreenViewport);
    gSPSetGeometryMode(renderState->dl++, G_ZBUFFER);

    // gDPSetColorImage(renderState->dl++, G_IM_FMT_RGBA, G_IM_SIZ_16b, SCREEN_WD, osVirtualToPhysical(gfxTask->framebuffer));

    shadowMap->nearPlane = nearPlane * SCENE_SCALE;
    shadowMap->projOffset = projOffset;
}

#define DEBUG_X 32
#define DEBUG_Y 32

void shadowMapRenderDebug(struct RenderState* renderState, u16* buffer) {
    gDPPipeSync(renderState->dl++);
    gDPSetCycleType(renderState->dl++, G_CYC_1CYCLE);
    gDPSetTextureLUT(renderState->dl++, G_TT_NONE);
    gDPSetRenderMode(renderState->dl++, G_RM_OPA_SURF, G_RM_OPA_SURF2);
    gDPSetTexturePersp(renderState->dl++, G_TP_NONE);
    gDPSetCombineLERP(renderState->dl++, 0, 0, 0, TEXEL0, 0, 0, 0, 1, 0, 0, 0, TEXEL0, 0, 0, 0, 1);
    gDPTileSync(renderState->dl++);
    gDPLoadTextureTile(
        renderState->dl++,
        buffer,
        G_IM_FMT_I, G_IM_SIZ_8b,
        SHADOW_MAP_WIDTH, SHADOW_MAP_HEIGHT,
        0, 0,
        SHADOW_MAP_WIDTH-1, SHADOW_MAP_HEIGHT-1,
        0,
        G_TX_CLAMP | G_TX_NOMIRROR, G_TX_CLAMP | G_TX_NOMIRROR,
        0, 0, 
        0, 0
    );
    gSPTextureRectangle(
        renderState->dl++, 
        DEBUG_Y << 2, DEBUG_X << 2, 
        (DEBUG_X + SHADOW_MAP_WIDTH) << 2, (DEBUG_Y + SHADOW_MAP_HEIGHT) << 2, 
        G_TX_RENDERTILE, 
        0, 0, 
        1 << 10, 1 << 10
    );
}

void shadowMapInit(struct ShadowMap* shadowMap, float radius, u16* buffer) {
    shadowMap->buffer = buffer;
    shadowMap->subjectRadius = radius;
    shadowMap->flags = 0;
}
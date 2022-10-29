#ifndef __RENDER_STATE_H__
#define __RENDER_STATE_H__

#include <ultra64.h>

#include "color.h"

#define MAX_DL_LENGTH           6000
#define MAX_RENDER_STATE_MEMORY 12000
#define MAX_RENDER_STATE_MEMORY_CHUNKS (MAX_RENDER_STATE_MEMORY / sizeof(u64))
#define MAX_DYNAMIC_LIGHTS      128
#define MAX_LAYER_COUNT     16

struct SpriteState {
    Gfx* layerSetup[MAX_LAYER_COUNT];
    Gfx* layerRevert[MAX_LAYER_COUNT];
    struct Coloru8 layerColor[MAX_LAYER_COUNT];
    Gfx* layerDL[MAX_LAYER_COUNT];
    Gfx* currentLayerDL[MAX_LAYER_COUNT];
    Gfx* currentChunkEnd[MAX_LAYER_COUNT];
};

struct RenderState {
    Gfx glist[MAX_DL_LENGTH];
    u64 renderStateMemory[MAX_RENDER_STATE_MEMORY_CHUNKS];
    struct SpriteState spriteState;
    Gfx* dl;
    u16* framebuffer;
    u16* depthBuffer;
    unsigned short currentMemoryChunk;
    unsigned short currentChunkEnd;
};

void renderStateInit(struct RenderState* renderState, u16* framebuffer, u16* depthBuffer);
void* renderStateRequestMemory(struct RenderState* renderState, unsigned size);
Mtx* renderStateRequestMatrices(struct RenderState* renderState, unsigned count);
Light* renderStateRequestLights(struct RenderState* renderState, unsigned count);
Vp* renderStateRequestViewport(struct RenderState* renderState);
Vtx* renderStateRequestVertices(struct RenderState* renderState, unsigned count);
LookAt* renderStateRequestLookAt(struct RenderState* renderState);
void renderStateFlushCache(struct RenderState* renderState);
Gfx* renderStateAllocateDLChunk(struct RenderState* renderState, unsigned count);
Gfx* renderStateReplaceDL(struct RenderState* renderState, Gfx* nextDL);
Gfx* renderStateStartChunk(struct RenderState* renderState);
Gfx* renderStateEndChunk(struct RenderState* renderState, Gfx* chunkStart);

#endif
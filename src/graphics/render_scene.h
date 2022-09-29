#ifndef __RENDER_SCENE_H__
#define __RENDER_SCENE_H__

#include <ultra64.h>
#include "../math/transform.h"
#include "../math/plane.h"
#include "renderstate.h"

struct RenderPart {
    Mtx* matrix;
    Gfx* geometry;
    Mtx* armature;
    Light* light;
};

struct RenderScene {
    u64 visibleRooms;
    struct Transform cameraTransform;
    struct Plane forwardPlane;
    struct RenderPart* renderParts;
    short* materials;
    int* sortKeys;
    short* renderOrder;
    short* renderOrderCopy;
    int currentRenderPart;
    int maxRenderParts;
    struct RenderState *renderState;
    Gfx* defaultMaterial;
};

struct RenderScene* renderSceneNew(struct Transform* cameraTransform, struct RenderState *renderState, int capacity, u64 visibleRooms, Gfx* defaultMaterial);
void renderSceneFree(struct RenderScene* renderScene);
void renderSceneAdd(struct RenderScene* renderScene, Gfx* geometry, Mtx* matrix, int materialIndex, struct Vector3* at, Mtx* armature, Light* light);
void renderSceneGenerate(struct RenderScene* renderScene, struct RenderState* renderState);

#define RENDER_SCENE_IS_ROOM_VISIBLE(renderScene, roomIndex) (((renderScene)->visibleRooms & (1 << (roomIndex))) != 0)

#endif
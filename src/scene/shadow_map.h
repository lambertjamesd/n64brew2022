#ifndef __SHADOW_MAP_H__
#define __SHADOW_MAP_H__

#include <ultra64.h>
#include "graphics/graphics.h"
#include "math/transform.h"
#include "math/plane.h"
#include "shadow_map.h"
#include "point_light.h"

#define SHADOW_MAP_WIDTH    64
#define SHADOW_MAP_HEIGHT   64

#define SHADOW_MAP_ENABLED  (1 << 0)

struct ShadowMap {
    struct Transform lightPovTransform;
    float subjectRadius;
    float nearPlane;
    float farPlane;
    float projOffset;
    u16* buffer;
    u32 flags;
};

void shadowMapInit(struct ShadowMap* shadowMap, float radius, float nearPlane, float farPlane, u16* buffer);
void shadowMapRender(struct ShadowMap* shadowMap, struct RenderState* renderState, struct GraphicsTask* gfxTask, struct Vector3* from, struct Transform* subjectTransform, Gfx* subject);
void shadowMapRenderDebug(struct RenderState* renderState, u16* buffer);
void shadowMapRenderOntoPlane(struct ShadowMap* shadowMap, struct RenderState* renderState, struct Plane* ontoPlane);


#endif
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

struct ShadowMap {
    float subjectRadius;
    u16* buffer;
};

void shadowMapInit(struct ShadowMap* shadowMap, float radius, u16* buffer);
void shadowMapRender(struct ShadowMap* shadowMap, struct RenderState* renderState, struct GraphicsTask* gfxTask, struct Vector3* from, struct Transform* subjectTransform, Gfx* subject, struct Plane* onto);
void shadowMapRenderDebug(struct RenderState* renderState, u16* buffer);


#endif
#ifndef __SCENE_TABLE_SURFACE_H__
#define __SCENE_TABLE_SURFACE_H__

#include "table_type.h"
#include "spot_light.h"
#include "../graphics/renderstate.h"

void tableSurfaceRenderLight(struct TableSurfaceMesh* surface, struct Vector3* offset, struct SpotLight* spotLight, struct RenderState* renderState);

void tableSurfaceRenderShadow(struct TableSurfaceMesh* surface, struct Vector3* offset, struct SpotLight* spotLight, struct RenderState* renderState);

#endif
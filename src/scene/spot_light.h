#ifndef __SPOT_LIGHT_H__
#define __SPOT_LIGHT_H__

#include "../math/transform.h"
#include "../level/level_definition.h"
#include "../graphics/renderstate.h"

struct SpotLight {
    struct Transform transform;
    float horizontalScale;
};

void spotLightSetAngle(struct SpotLight* spotLight, float angle);

void spotLightInit(struct SpotLight* spotLight, struct SpotLightDefinition* spotLightDef);

void spotLightRenderProjection(struct SpotLight* spotLight, struct RenderState* renderState);

#endif
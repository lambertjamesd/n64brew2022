#ifndef __SPOT_LIGHT_H__
#define __SPOT_LIGHT_H__

#include "../math/transform.h"
#include "../level/level_definition.h"
#include "../graphics/renderstate.h"
#include "../collision/collision_object.h"
#include "../math/box3d.h"

#define LIGHT_CIRCLE_POINT_COUNT    8

enum LightIntersection {
    LightIntersectionOutside = 0,
    LightIntersectionTouchingBackFace = (1 << 0),
    LightIntersectionTouchingFrontFace = (1 << 1),
    LightIntersectionInside = (1 << 2),
};

struct SpotLight {
    struct Transform transform;
    struct Vector3 lightOutline[LIGHT_CIRCLE_POINT_COUNT];
    struct Vector3 faceNormal[LIGHT_CIRCLE_POINT_COUNT];
    struct Vector3 centerDirection;
    float borderDot;
    struct Box3D boundingBox;
    int isBackFaceMask;
    float horizontalScale;
};

void spotLightSetAngle(struct SpotLight* spotLight, float angle);

void spotLightInit(struct SpotLight* spotLight, struct SpotLightDefinition* spotLightDef, struct Vector3* cameraPos);

void spotLightUpdate(struct SpotLight* spotLight, struct Vector3* cameraPos);

void spotLightRenderProjection(struct SpotLight* spotLight, struct RenderState* renderState);

float spotLightClosenessWeight(struct SpotLight* spotLight, struct Vector3* point);
enum LightIntersection spotLightPointIsInside(struct SpotLight* spotLight, struct Vector3* point);
enum LightIntersection spotLightIsInside(struct SpotLight* spotLight, struct CollisionObject* collisionObject);

struct LightConfiguration {
    struct SpotLight* primaryLight;
    struct SpotLight* secondaryLight;
    float blendWeight;
};

int spotLightsFindConfiguration(struct SpotLight* lights, int lightCount, struct Vector3* point, struct CollisionObject* collisionObject, struct LightConfiguration* output);

void spotLightsSetupLight(struct LightConfiguration* lightConfig, struct Vector3* target, struct RenderState* renderState);

#endif
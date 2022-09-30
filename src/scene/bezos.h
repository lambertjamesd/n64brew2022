#ifndef __SCENE_BEZOS_H__
#define __SCENE_BEZOS_H__

#include "../math/transform.h"
#include "../sk64/skelatool_animator.h"
#include "../sk64/skelatool_armature.h"
#include "./spot_light.h"

#include "../graphics/render_scene.h"

enum BezosFlags {
    BezosFlagsActive = (1 << 0),
};

struct Bezos {
    struct Transform transform;
    struct SKAnimator animator;
    struct SKArmature armature;
    short flags;
};

void bezosInit(struct Bezos* bezos);

void bezosActivate(struct Bezos* bezos, struct Vector3* at);
void bezosDeactivate(struct Bezos* bezos);

void bezosUpdate(struct Bezos* bezos);
void bezosRender(struct Bezos* bezos, struct SpotLight* spotLights, int spotLightCount, struct RenderScene* renderScene);

#endif
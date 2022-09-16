#ifndef __SCENE_ITEM_H__
#define __SCENE_ITEM_H__

#include "../sk64/skelatool_animator.h"
#include "../sk64/skelatool_armature.h"

#include "../math/transform.h"
#include "../level/level_definition.h"

enum ItemType {
    ItemTypePumpkin,
};

struct Item {
    enum ItemType type;
    struct Transform transform;
    struct SKAnimator animator;
    struct SKArmature armature;
};

void itemInit(struct Item* item);

#endif
#ifndef __SCENE_CONVEYOR_TYPE_H__
#define __SCENE_CONVEYOR_TYPE_H__

#include <ultra64.h>

#include "../math/box3d.h"

struct ConveyorType {
    Gfx* displayList;
    struct Box3D boundingBox;
    short materialIndex;
};

#endif
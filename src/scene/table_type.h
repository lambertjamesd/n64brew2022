#ifndef __SCENE_TABLE_TYPE_H__
#define __SCENE_TABLE_TYPE_H__

#include <ultra64.h>

#include "../math/box3d.h"
#include "../math/vector3.h"

struct TableType {
    Gfx* displayList;
    struct Box3D boundingBox;
    struct Vector3* itemSlots;
    short itemSlotCount;
    short materialIndex;
};

#endif
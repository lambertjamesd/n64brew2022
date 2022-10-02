#ifndef __COLLISION_OBJECT_H__
#define __COLLISION_OBJECT_H__

#include "gjk.h"
#include "../math/box3d.h"

struct CollisionObject {
    struct Box3D boundingBox;
    MinkowsiSum minkowskiSum;
    void* data;
};

#endif
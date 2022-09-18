#ifndef __COLLISION_OBJECT_H__
#define __COLLISION_OBJECT_H__

#include "gjk.h"
#include "../math/box3d.h"

#define COLLISION_OBJECT_DATA_POINTER(collisionObject) (void*)((char*)(collisionObject) + (collisionObject)->dataPointerOffset)
#define COLLISION_OBJECT_OFFSET(data, collisionObject) ((int)(data) - (int)(collisionObject))

struct CollisionObject {
    struct Box3D boundingBox;
    MinkowsiSum minkowskiSum;
    int dataPointerOffset;
};

#endif
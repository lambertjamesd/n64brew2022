#ifndef __COLLISION_OBJECT_H__
#define __COLLISION_OBJECT_H__

#include "gjk.h"
#include "../math/box3d.h"

struct CollisionObject {
    struct Box3D boundingBox;
    MinkowsiSum minkowskiSum;
    void* data;
};

int collisionObjectBoundingBox(void* data, struct Vector3* direction, struct Vector3* output);

struct CollisionCapsule {
    struct CollisionObject collisionObject;
    struct Vector3 center;
    float halfHeight;
    float radius;
};

void collisionCapsuleUpdateBB(struct CollisionCapsule* capsule);

void collisionCapsuleInit(struct CollisionCapsule* capsule, float height, float radius);

#endif
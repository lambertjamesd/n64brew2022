#ifndef __COLLISION_COLLIISION_BOUNDARY_H__
#define __COLLISION_COLLIISION_BOUNDARY_H__

#include "../math/vector2.h"
#include "./collision_object.h"

struct CollisionBoundary {
    struct CollisionObject collisionObject;
    struct Vector2 a;
    struct Vector2 b;
};

void collisionBoundaryInit(struct CollisionBoundary* boundary, struct Vector2* a, struct Vector2* b);

#endif
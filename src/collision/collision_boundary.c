#include "collision_boundary.h"

#include <ultra64.h>
#define WALL_HEIGHT     5.0f

int collisionBoundarySum(void* data, struct Vector3* direction, struct Vector3* output) {
    struct CollisionBoundary* boundary = (struct CollisionBoundary*)data;
    int result = 0;

    if (direction->x * boundary->a.x + direction->z * boundary->a.y > direction->x * boundary->b.x + direction->z * boundary->b.y) {
        output->x = boundary->a.x;
        output->z = boundary->a.y;
    } else {
        output->x = boundary->b.x;
        output->z = boundary->b.y;
    }

    if (direction->y < 0.0f) {
        output->y = 0.0f;
    } else {
        output->y = WALL_HEIGHT;
    }

    return result;
}

void collisionBoundaryInit(struct CollisionBoundary* boundary, struct Vector2* a, struct Vector2* b) {
    boundary->a = *a;
    boundary->b = *b;

    boundary->collisionObject.boundingBox.min.x = MIN(a->x, b->x);
    boundary->collisionObject.boundingBox.min.y = 0.0f;
    boundary->collisionObject.boundingBox.min.z = MIN(a->y, b->y);

    boundary->collisionObject.boundingBox.max.x = MAX(a->x, b->x);
    boundary->collisionObject.boundingBox.max.y = WALL_HEIGHT;
    boundary->collisionObject.boundingBox.max.z = MAX(a->y, b->y);

    boundary->collisionObject.data = boundary;
    boundary->collisionObject.minkowskiSum = collisionBoundarySum;
}
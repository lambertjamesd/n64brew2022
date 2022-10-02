#include "collision_object.h"

int collisionObjectBoundingBox(void* data, struct Vector3* direction, struct Vector3* output) {
    struct CollisionObject* collisionObject = (struct CollisionObject*)data;

    int result = 0;

    if (direction->x > 0.0) {
        output->x = collisionObject->boundingBox.max.x;
        result |= (1 << 0);
    } else {
        output->x = collisionObject->boundingBox.min.x;
        result |= (1 << 1);
    }

    if (direction->y > 0.0) {
        output->y = collisionObject->boundingBox.max.y;
        result |= (1 << 2);
    } else {
        output->y = collisionObject->boundingBox.min.y;
        result |= (1 << 3);
    }

    if (direction->z > 0.0) {
        output->z = collisionObject->boundingBox.max.z;
        result |= (1 << 4);
    } else {
        output->z = collisionObject->boundingBox.min.z;
        result |= (1 << 5);
    }

    return result;
}

int collisionCapsuleMinkowsi(void* data, struct Vector3* direction, struct Vector3* output) {
    struct CollisionCapsule* capsule = (struct CollisionCapsule*)data;

    vector3AddScaled(&capsule->center, direction, capsule->radius, output);

    if (direction->y > 0.0f) {
        output->y += capsule->halfHeight;
    } else {
        output->y -= capsule->halfHeight;
    }

    return 1;
}

void collisionCapsuleUpdateBB(struct CollisionCapsule* capsule) {
    capsule->collisionObject.boundingBox.min.x = capsule->center.x - capsule->radius;
    capsule->collisionObject.boundingBox.min.y = capsule->center.y - capsule->radius - capsule->halfHeight;
    capsule->collisionObject.boundingBox.min.z = capsule->center.z - capsule->radius;

    capsule->collisionObject.boundingBox.max.x = capsule->center.x + capsule->radius;
    capsule->collisionObject.boundingBox.max.y = capsule->center.y + capsule->radius + capsule->halfHeight;
    capsule->collisionObject.boundingBox.max.z = capsule->center.z + capsule->radius;
}

void collisionCapsuleInit(struct CollisionCapsule* capsule, float height, float radius) {
    capsule->collisionObject.minkowskiSum = collisionCapsuleMinkowsi;
    capsule->collisionObject.data = capsule;

    capsule->center = gZeroVec;
    capsule->halfHeight = height * 0.5f;
    capsule->radius = radius;
}
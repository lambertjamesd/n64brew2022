
#include "box3d.h"

int box3DContainsPoint(struct Box3D* box, struct Vector3* point) {
    return box->min.x < point->x &&
        box->min.y < point->y &&
        box->min.z < point->z &&
        box->max.x > point->x &&
        box->max.y > point->y &&
        box->max.z > point->z;
        
}

int box3DHasOverlap(struct Box3D* a, struct Box3D* b) {
    return a->min.x <= b->max.x && a->max.x >= b->min.x &&
        a->min.y <= b->max.y && a->max.y >= b->min.y &&
        a->min.z <= b->max.z && a->max.z >= b->min.z;
}

void box3DUnion(struct Box3D* a, struct Box3D* b, struct Box3D* out) {
    vector3Max(&a->max, &b->max, &out->max);
    vector3Min(&a->min, &b->min, &out->min);
}

void box3DRotate(struct Box3D* input, struct Quaternion* rotation, struct Box3D* out) {
    struct Vector3 center;
    vector3Lerp(&input->min, &input->max, 0.5f, &center);
    struct Vector3 halfSize;
    vector3Sub(&center, &input->min, &halfSize);

    struct Vector3 rotatedHalfSize;
    quatRotatedBoundingBoxSize(rotation, &halfSize, &rotatedHalfSize);
    vector3Add(&center, &rotatedHalfSize, &out->max);
    vector3Sub(&center, &rotatedHalfSize, &out->min);
}

void box3DNearestPoint(struct Box3D* input, struct Vector3* point, struct Vector3* output) {
    if (point->x > input->max.x) {
        output->x = input->max.x;
    } else if (point->x < input->min.x) {
        output->x = input->min.x;
    } else {
        output->x = point->x;
    }

    if (point->y > input->max.y) {
        output->y = input->max.y;
    } else if (point->y < input->min.y) {
        output->y = input->min.y;
    } else {
        output->y = point->y;
    }

    if (point->z > input->max.z) {
        output->z = input->max.z;
    } else if (point->z < input->min.z) {
        output->z = input->min.z;
    } else {
        output->z = point->z;
    }
}

void box3DOffset(struct Box3D* input, struct Vector3* offset, struct Box3D* ouput) {
    vector3Add(&input->min, offset, &ouput->min);
    vector3Add(&input->max, offset, &ouput->max);
}

void box3DExtend(struct Box3D* input, float amount, struct Box3D* output) {
    output->min.x = input->min.x - amount;
    output->min.y = input->min.y - amount;
    output->min.z = input->min.z - amount;

    output->max.x = input->max.x + amount;
    output->max.y = input->max.y + amount;
    output->max.z = input->max.z + amount;
}
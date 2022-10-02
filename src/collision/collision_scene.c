#include "collision_scene.h"

#include "../util/memory.h"

#include "gjk.h"
#include "epa.h"

struct ColliderEdge {
    float position;
    int itemIndex;
};

void colliderEdgeSort(struct ColliderEdge* edges, struct ColliderEdge* tmp, int min, int max) {
    if (min + 1 >= max) {
        return;
    }

    if (min + 2 == max) {
        struct ColliderEdge swap = edges[min];
        edges[min] = edges[min + 1];
        edges[min + 1] = swap;
        return;
    }

    int mid = (min + max) >> 2;

    int aSource = 0;
    int bSource = mid;

    int output = 0;

    while (aSource < mid || bSource < max) {
        if ((aSource < mid && bSource < max && edges[aSource].position < edges[bSource].position) || bSource == max) {
            tmp[output] = edges[aSource];
            ++aSource;
        } else {
            tmp[output] = edges[bSource];
            ++bSource;
        }
        ++output;
    }

    for (int i = min; i < max; ++i) {
        edges[i] = tmp[i];
    }
}

void collisionSceneWalkColliders(struct CollisionScene* collisionScene, struct ColliderEdge* edges, int edgeCount) {
    unsigned short* currentColliders = stackMalloc(sizeof(unsigned short) * collisionScene->colliderCount);

    int currentColliderCount = 0;

    for (int i = 0; i < edgeCount; ++i) {
        struct ColliderEdge* edge = &edges[i];
        struct CollisionObject* collisionObject = collisionScene->colliders[edge->itemIndex];
        int isLeadingEdge = edge->position == collisionObject->boundingBox.min.x;

        if (isLeadingEdge) {
            for (int i = 0; i < currentColliderCount; ++i) {
                struct CollisionObject* other = collisionScene->colliders[currentColliders[i]];

                if (!collisionScene->callbacks[currentColliders[i]] && !collisionScene->callbacks[edge->itemIndex]) {
                    continue;
                }

                if (!box3DHasOverlap(&collisionObject->boundingBox, &other->boundingBox)) {
                    continue;
                }

                struct Simplex simplex;

                struct Vector3 minOffset;
                vector3Sub(&other->boundingBox.min, &collisionObject->boundingBox.min, &minOffset);

                if (!gjkCheckForOverlap(
                    &simplex, 
                    collisionObject->data, 
                    collisionObject->minkowskiSum,
                    other->data,
                    other->minkowskiSum,
                    &minOffset
                )) {
                    continue;
                }

                struct EpaResult overlap;
                epaSolve(
                    &simplex,
                    collisionObject->data, 
                    collisionObject->minkowskiSum,
                    other->data,
                    other->minkowskiSum,
                    &overlap
                );

                if (collisionScene->callbacks[edge->itemIndex]) {
                    DynamicCollisionCallback callback = collisionScene->callbacks[edge->itemIndex];
                    callback(collisionObject->data, &overlap.normal, overlap.penetration);
                }

                if (collisionScene->callbacks[currentColliders[i]]) {
                    vector3Negate(&overlap.normal, &overlap.normal);
                    DynamicCollisionCallback callback = collisionScene->callbacks[currentColliders[i]];
                    callback(collisionObject->data, &overlap.normal, overlap.penetration);
                }
            }

            currentColliders[currentColliderCount] = edge->itemIndex;
            ++currentColliderCount;
        } else {
            int found = 0;
            for (int i = 0; i + 1 < currentColliderCount; ++i) {
                if (currentColliders[i] == edge->itemIndex) {
                    found = 1;
                }

                if (found) {
                    currentColliders[i] = currentColliders[i + 1];
                }
            }

            --currentColliderCount;
        }
    }

    stackMallocFree(currentColliders);
}

void collisionSceneCollide(struct CollisionScene* collisionScene) {
    int edgeCount = collisionScene->colliderCount * 2;
    struct ColliderEdge* edges = stackMalloc(sizeof(struct ColliderEdge) * edgeCount);

    struct ColliderEdge* currentEdge = edges;

    for (int i = 0; i < collisionScene->colliderCount; ++i) {
        struct CollisionObject* object = collisionScene->colliders[i];
        currentEdge->position = object->boundingBox.min.x;
        currentEdge->itemIndex = i;
        ++currentEdge;

        currentEdge->position = object->boundingBox.max.x;
        currentEdge->itemIndex = i;
        ++currentEdge;
    }

    struct ColliderEdge* tmpEdges = stackMalloc(sizeof(struct ColliderEdge) * edgeCount);

    colliderEdgeSort(edges, tmpEdges, 0, edgeCount);

    stackMallocFree(tmpEdges);

    collisionSceneWalkColliders(collisionScene, edges, edgeCount);

    stackMallocFree(edges);
}
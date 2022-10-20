#include "collision_scene.h"

#include "../util/memory.h"

#include "gjk.h"
#include "epa.h"

struct ColliderEdge {
    float position;
    int itemIndex;
};


struct CollisionScene gCollisionScene;

void collisionSceneInit(struct CollisionScene* collisionScene, int capacity) {
    collisionScene->colliders = malloc(sizeof(struct CollisionObject*) * capacity);
    collisionScene->callbacks = malloc(sizeof(struct DynamicCallbackPair) * capacity);
    collisionScene->colliderCapacity = capacity;
    collisionScene->colliderCount = 0;
}

void collisionSceneAddStatic(struct CollisionScene* collisionScene, struct CollisionObject* object) {
    if (collisionScene->colliderCount == collisionScene->colliderCapacity) {
        return;
    }    

    collisionScene->colliders[collisionScene->colliderCount] = object;
    collisionScene->callbacks[collisionScene->colliderCount].callback = NULL;
    ++collisionScene->colliderCount;
}

void collisionSceneAddDynamic(struct CollisionScene* collisionScene, struct CollisionObject* object, DynamicCollisionCallback collisionCallback, void* data) {
    if (collisionScene->colliderCount == collisionScene->colliderCapacity) {
        return;
    }    

    collisionScene->colliders[collisionScene->colliderCount] = object;
    collisionScene->callbacks[collisionScene->colliderCount].callback = collisionCallback;
    collisionScene->callbacks[collisionScene->colliderCount].data = data;
    ++collisionScene->colliderCount;
}


void colliderEdgeSort(struct ColliderEdge* edges, struct ColliderEdge* tmp, int min, int max) {
    if (min + 1 >= max) {
        return;
    }

    if (min + 2 == max) {
        if (edges[min].position > edges[min + 1].position) {
            struct ColliderEdge swap = edges[min];
            edges[min] = edges[min + 1];
            edges[min + 1] = swap;
        }
        return;
    }

    int mid = (min + max) >> 1;

    colliderEdgeSort(edges, tmp, min, mid);
    colliderEdgeSort(edges, tmp, mid, max);

    int aSource = min;
    int bSource = mid;

    int output = min;

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

                if (!collisionScene->callbacks[currentColliders[i]].callback && !collisionScene->callbacks[edge->itemIndex].callback) {
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

                if (collisionScene->callbacks[currentColliders[i]].callback) {
                    struct DynamicCallbackPair callback = collisionScene->callbacks[currentColliders[i]];
                    callback.callback(callback.data, &overlap.normal, -overlap.penetration, collisionObject);
                }

                if (collisionScene->callbacks[edge->itemIndex].callback) {
                    vector3Negate(&overlap.normal, &overlap.normal);
                    struct DynamicCallbackPair callback = collisionScene->callbacks[edge->itemIndex];
                    callback.callback(callback.data, &overlap.normal, -overlap.penetration, other);
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
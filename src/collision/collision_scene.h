#ifndef __COLLISION_COLLISION_SCENE_H__
#define __COLLISION_COLLISION_SCENE_H__

#include "collision_object.h"

typedef void (*DynamicCollisionCallback)(void* data, struct Vector3* normal, float depth);

struct CollisionScene {
    struct CollisionObject** colliders;
    DynamicCollisionCallback* callbacks;
    short colliderCount;
    short colliderCapacity;

};

void collisionSceneInit(struct CollisionScene* collisionScene, int capacity);

void collisionSceneAddStatic(struct CollisionScene* collisionScene, struct CollisionObject* object);
void collisionSceneAddDynamic(struct CollisionScene* collisionScene, struct CollisionObject* object, DynamicCollisionCallback collisionCallback);


void collisionSceneCollide(struct CollisionScene* collisionScene);

#endif
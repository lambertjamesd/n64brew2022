#ifndef __COLLISION_COLLISION_SCENE_H__
#define __COLLISION_COLLISION_SCENE_H__

#include "collision_object.h"

typedef void (*DynamicCollisionCallback)(void* data, struct Vector3* normal, float depth);

struct DynamicCallbackPair {
    DynamicCollisionCallback callback;
    void* data;
};

struct CollisionScene {
    struct CollisionObject** colliders;
    struct DynamicCallbackPair* callbacks;
    short colliderCount;
    short colliderCapacity;
};

extern struct CollisionScene gCollisionScene;

void collisionSceneInit(struct CollisionScene* collisionScene, int capacity);

void collisionSceneAddStatic(struct CollisionScene* collisionScene, struct CollisionObject* object);
void collisionSceneAddDynamic(struct CollisionScene* collisionScene, struct CollisionObject* object, DynamicCollisionCallback collisionCallback, void* data);

void collisionSceneCollide(struct CollisionScene* collisionScene);

#endif
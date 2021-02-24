#pragma once
#include <btBulletDynamicsCommon.h>
#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>


class CollisionObject {
protected:
	btRigidBody* meshBody = nullptr;
public:
	CollisionObject() {}
	virtual ~CollisionObject(){}


};
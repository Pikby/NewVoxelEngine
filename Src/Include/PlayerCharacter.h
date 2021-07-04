#include <glm/glm.hpp>
#include <glad/glad.h> 
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/detail/compute_vector_relational.hpp>
#include <btBulletDynamicsCommon.h>

#include "Entity.h"
#include "Lighting.h"
class PlayerCharacter : public Entity{
private:


public:
	PlayerCharacter(const glm::vec3& Pos) {
		pointLightFlag = false;
		shape = std::make_unique<btCapsuleShape>(1,3);
		btVector3 inertia = btVector3(0.0, 0.0, 0.0);
		shape->calculateLocalInertia(2.0f, inertia);

		object = std::make_unique<btCollisionObject>();
		object->setCollisionShape(shape.get());

		glm::mat4 model = glm::translate(glm::mat4(1), Pos);

		btTransform transform;
		transform.setFromOpenGLMatrix((float*)&model);

		motionState = std::make_unique<btDefaultMotionState>(transform);
		btRigidBody::btRigidBodyConstructionInfo rbInfo(1.0f, motionState.get(), shape.get(), inertia);
		body = std::make_unique<btRigidBody>(rbInfo);

	}

	void draw(Shader& shader, const Camera& camera) {

	}

};
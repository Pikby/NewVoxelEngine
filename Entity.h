#pragma once
#include <glad/glad.h> 
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <btBulletDynamicsCommon.h>

#include "Shader.h"
#include "Camera.h"
#include "Model.h"
class Entity {
private:
	std::unique_ptr<btCollisionShape> shape;
	std::unique_ptr<btCollisionObject> object;
	std::unique_ptr<btDefaultMotionState> motionState;
	std::unique_ptr<btRigidBody> body;

	float size = 0.5;
public:
	Entity(const glm::vec3& Pos, const glm::vec3& Facing){
		shape = std::make_unique<btSphereShape>(size);
		btVector3 inertia = btVector3(0.0, 0.0, 0.0);
		shape->calculateLocalInertia(2.0f, inertia);

		object = std::make_unique<btCollisionObject>();
		object->setCollisionShape(shape.get());

		glm::mat4 model = glm::translate(glm::mat4(1), Pos);

		btTransform transform;
		transform.setFromOpenGLMatrix((float*)&model);

		motionState = std::make_unique<btDefaultMotionState>(transform);
		btRigidBody::btRigidBodyConstructionInfo rbInfo(1.0f, motionState.get(), shape.get(), inertia);
		body = std::make_unique<btRigidBody> (rbInfo);
		body->setRestitution(0.5);
		body->setFriction(1.0);
		body->setRollingFriction(.1);
		body->setSpinningFriction(0.1);
	}

	glm::vec3 getPosition() {
		btTransform transform = body->getWorldTransform();

		return glm::vec3(transform.getOrigin().getX(), transform.getOrigin().getY(),transform.getOrigin().getZ());
	}


	btRigidBody* getRigidBody() {
		return body.get();
	}

	virtual void draw(Shader& shader, const Camera& camera) {
		struct QuadVertex {
			glm::vec3 pos;
			glm::vec2 tex;
		};
		static QuadVertex quadVertices[] = {
			// positions          // colors      // texture coords
			{{1.0f,  1.0f, 0.0f},   {1.0f, 1.0f}},   // top right
			{{1.0f, -1.0f, 0.0f},   {1.0f, 0.0f}},   // bottom right
			{{-1.0f, -1.0f, 0.0f},  {0.0f, 0.0f}},   // bottom left

			{{1.0f,  1.0f, 0.0f},   {1.0f, 1.0f}},   // top right
			{{-1.0f, -1.0f, 0.0f},  {0.0f, 0.0f}},   // bottom left
			{{-1.0f,  1.0f, 0.0f},  {0.0f, 1.0f}},    // top left 
		};

		static unsigned int VAO = 0, VBO = 0;
		if (VAO == 0) {
			glGenVertexArrays(1, &VAO);
			glGenBuffers(1, &VBO);

			glBindVertexArray(VAO);
			glBindBuffer(GL_ARRAY_BUFFER, VBO);
			glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(QuadVertex), quadVertices, GL_STATIC_DRAW);

			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(decltype(quadVertices[0])), (void*)(offsetof(QuadVertex, pos)));

			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(decltype(quadVertices[0])), (void*)(offsetof(QuadVertex, tex)));
			glBindVertexArray(0);
		}
		glm::mat4 model(1);
		model = glm::translate(model, getPosition());
		model = glm::scale(model, glm::vec3(0.01));

		shader.setMat4("model", model);
		shader.setMat4("view", camera.getViewMatrix());
		shader.setFloat("time", glfwGetTime());
		shader.setFloat("cloudSpeed", 50);
		shader.setVec2("windDirection", glm::normalize(glm::vec2(1, 0)));

	
		static Model bunny("bunny.obj");
		//bunny.Draw(shader);
		glDisable(GL_CULL_FACE);
		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glBindVertexArray(0);
		glEnable(GL_CULL_FACE);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	float getSize() {
		return size;
	}
};
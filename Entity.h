#pragma once
#include <glad/glad.h> 
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <btBulletDynamicsCommon.h>

#include "Shader.h"
#include "Camera.h"
#include "Model.h"
#include "Common.h"
#include "GlobalLighting.h"
class Entity {
protected:
	std::unique_ptr<btCollisionShape> shape;
	std::unique_ptr<btCollisionObject> object;
	std::unique_ptr<btDefaultMotionState> motionState;
	std::unique_ptr<btRigidBody> body;

	float size = 0.5;
	bool pointLightFlag = false;
public:
	Entity(){};
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

	glm::mat4 getModelMatrix() {
		btTransform transform = body->getWorldTransform();

		glm::mat4 model;
		transform.getOpenGLMatrix(glm::value_ptr(model));
		return model;
	}

	btRigidBody* getRigidBody() {
		return body.get();
	}

	bool hasPointLight() {
		return pointLightFlag;
	}

	virtual PointLight* getPointLight() {
		return nullptr;
	}

	virtual void draw(Shader& shader, const Camera& camera) {
		
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
		OpenGLCommon::drawQuad();
		glEnable(GL_CULL_FACE);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	float getSize() {
		return size;
	}
};

class Torch : public Entity {
private:
	PointLight pointLight;
public:
	Torch(const glm::vec3& Pos) {

		pointLightFlag = true;
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
		body = std::make_unique<btRigidBody>(rbInfo);
		body->setRestitution(0.5);
		body->setFriction(1.0);
		body->setRollingFriction(.1);
		body->setSpinningFriction(0.1);
	}


	void draw(Shader& shader, const Camera& camera) {

		glm::mat4 model(1);
		model = glm::translate(model, getPosition());
		//model = glm::scale(model, glm::vec3(0.01));
		shader.use();
		shader.setMat4("model",getModelMatrix());
		static Model torch("torch.fbx");

		glCullFace(GL_FRONT);
		torch.Draw(shader);
		glCullFace(GL_BACK);
	}

	PointLight* getPointLight() {
		pointLight.setPosition(getPosition());
		return &pointLight;
	}
};
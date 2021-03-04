#pragma once
#include <glad/glad.h> 
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <btBulletDynamicsCommon.h>

#include "Shader.h"
#include "Camera.h"
#include "Model.h"
#include "Common.h"
#include "Lighting.h"
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
		body->setRollingFriction(.1f);
		body->setSpinningFriction(0.1f);
	}

	virtual ~Entity() {}

	glm::vec3 getPosition() const {
		btTransform transform = body->getWorldTransform();

		return glm::vec3(transform.getOrigin().getX(), transform.getOrigin().getY(),transform.getOrigin().getZ());
	}

	glm::mat4 getModelMatrix() const {
		btTransform transform = body->getWorldTransform();

		glm::mat4 model;
		transform.getOpenGLMatrix(glm::value_ptr(model));
		return model;
	}

	btRigidBody* getRigidBody() {
		return body.get();
	}

	bool hasPointLight() const{
		return pointLightFlag;
	}

	void setPosition(const glm::vec3 pos) {
		btTransform& transform = body->getWorldTransform();
		transform.setOrigin(btVector3(pos.x, pos.y, pos.z));
	}

	virtual PointLight* getPointLight() {
		return nullptr;
	}

	virtual void draw(Shader& shader, const Camera& camera) const {
		
		glm::mat4 model(1);
		model = glm::translate(model, getPosition());
		model = glm::scale(model, glm::vec3(0.01f));

		shader.setMat4("model", model);
		shader.setMat4("view", camera.getViewMatrix());
		shader.setFloat("time", float(glfwGetTime()));
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
		shape = std::make_unique<btCylinderShape>(btVector3(0.1f,1.0f,0.5f));
		btVector3 inertia = btVector3(0.0, 0.0, 0.0);
		shape->calculateLocalInertia(2.0f, inertia);

		object = std::make_unique<btCollisionObject>();
		object->setCollisionShape(shape.get());

		glm::mat4 model = glm::translate(glm::mat4(1), Pos-glm::vec3(0,4,0));
		model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0, 0, 1));
		btTransform transform;
		transform.setFromOpenGLMatrix((float*)&model);

		motionState = std::make_unique<btDefaultMotionState>(transform);
		btRigidBody::btRigidBodyConstructionInfo rbInfo(1.0f, motionState.get(), shape.get(), inertia);
		body = std::make_unique<btRigidBody>(rbInfo);
		body->setRestitution(0.5f);
		body->setFriction(1.0f);
		body->setRollingFriction(.1f);
		body->setSpinningFriction(0.1f);
	}


	void draw(Shader& shader, const Camera& camera) const override {

		static Shader Shader("Model.fs", "Model.vs");
		glm::mat4 model(1);
		model = getModelMatrix();
		model = glm::translate(model,glm::vec3(0.f, -0.8f, 0.f));
		model = glm::rotate(model, glm::radians(90.f), glm::vec3(-1, 0, 0));
		//model = glm::scale(model, glm::vec3(0.01));
		Shader.use();
		Shader.setMat4("model",model);
		static Model torch("Assets/torch.fbx");

		glCullFace(GL_FRONT);
		torch.draw(Shader);
		glCullFace(GL_BACK);
	}

	PointLight* getPointLight() override {
		btTransform transform = body->getWorldTransform();
		glm::mat4 model;
		transform.getOpenGLMatrix(glm::value_ptr(model));
		pointLight.setPosition(model*glm::vec4(0,1,0,1));
		return &pointLight;
	}
};

class Snowball : public Entity {

public:
	Snowball(const glm::vec3& Pos) {
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
		body->setRollingFriction(.1f);
		body->setSpinningFriction(0.1f);
	}

	void draw(Shader& SShader, const Camera& camera) const override {

		static Shader shader("Model.fs", "Model.vs");
		glm::mat4 model(1);
		model = getModelMatrix();
		model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));
		//model = glm::rotate(model, glm::radians(90.f), glm::vec3(-1, 0, 0));
		//model = glm::scale(model, glm::vec3(0.01));
		shader.use();
		shader.setMat4("model", model);
		shader.setVec3("objColor", glm::vec3(0.5));
		shader.setInt("translucent", 0);
		static Model torch("Assets/sphere.obj");

		//glCullFace(GL_FRONT);
		torch.draw(shader);
		//glCullFace(GL_BACK);
	}

};
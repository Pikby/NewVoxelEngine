#pragma once
#include <glm/glm.hpp>
#include "Shader.h"
#include "Common.h"

class Light {
public:
	glm::vec3 ambientColor;
	glm::vec3 diffuseColor;
	glm::vec3 specularColor;
	float specularExponent;


	unsigned int shadowTextureWidth = 4 * 1024;
	Light(const glm::vec3& AmbientColor = glm::vec3(0.5), const glm::vec3& DiffuseColor = glm::vec3(1), const glm::vec3& SpecularColor = glm::vec3(0.5), float SpecularExponent =32)
		: ambientColor(AmbientColor), diffuseColor(DiffuseColor), specularColor(SpecularColor), specularExponent(SpecularExponent) {}



	virtual void setShaderUniforms(Shader& shader) {};
	virtual void bindShadowBuffer() {};
	virtual void bindShadowTexture() {};
};



class DirectionalLight : public Light{
private:
	const unsigned shadowTextureOffset = 1;
	const double shadowWorldWidth = 500;

	unsigned int depthMapTexture = 0, depthMapFBO = 0, depthMap = 0, finalShadowTexture = 0;
	glm::vec3 globalLightDir = glm::vec3(0.1, 0.5, 0.1);

public:
	DirectionalLight() {}
	DirectionalLight(const glm::vec3& AmbientColor, const glm::vec3& DiffuseColor, const glm::vec3& SpecularColor, float SpecularExponent)
		: Light(AmbientColor,DiffuseColor, SpecularColor, SpecularExponent){}

	void setGlobaLightDirection(const glm::vec3& dir) {
		globalLightDir = dir;
	}

	void setShaderUniforms(Shader& shader) {
		shader.use();
		shader.setVec3("directionalLight.ambient", ambientColor);
		shader.setVec3("directionalLight.diffuse", diffuseColor);
		shader.setVec3("directionalLight.specular", specularColor);
		shader.setFloat("directionalLight.specularExponent", specularExponent);

		shader.setVec3("directionalLight.direction", globalLightDir);
		shader.setInt("directionalLight.shadowTexture", shadowTextureOffset);
	}



	void createShadowTexture(const int textureWidth, unsigned int& FBO, unsigned int& depthMap, unsigned int& depthMapTexture) {

		glGenFramebuffers(1, &FBO);

		glGenTextures(1, &depthMapTexture);
		glBindTexture(GL_TEXTURE_2D, depthMapTexture);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RG32F, textureWidth, textureWidth, 0, GL_RG, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

		glGenTextures(1, &depthMap);
		glBindTexture(GL_TEXTURE_2D, depthMap);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, textureWidth, textureWidth, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		//float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);


		glBindFramebuffer(GL_FRAMEBUFFER, FBO);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, depthMapTexture, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

	}


	void bindShadowBuffer() {
		if (depthMapFBO == 0) {
			createShadowTexture(shadowTextureWidth, depthMapFBO, depthMap, depthMapTexture);
		}
		glViewport(0, 0, shadowTextureWidth, shadowTextureWidth);
		glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
		glClear(GL_DEPTH_BUFFER_BIT);
	}

	void setDirectionalShadowMatrices(Shader& shader, glm::vec3 cameraPos) {

		const static glm::mat4 shadowProjection = glm::ortho(-shadowWorldWidth, shadowWorldWidth, -shadowWorldWidth, shadowWorldWidth, 1.0, shadowWorldWidth * 5);
		shader.use();
		shader.setMat4("shadowProjection",shadowProjection);
		glm::vec3 shadowDir = glm::vec3(1);
		glm::vec3 shadowOrigin = cameraPos + shadowDir*glm::vec3(2*shadowWorldWidth);
		glm::mat4 shadowView = glm::lookAt(shadowOrigin, shadowOrigin-glm::vec3(1,1,1), glm::vec3(0, 1, 0));
		shader.setMat4("shadowView", shadowView);
		
	}

	void bindShadowTexture() {
		glActiveTexture(GL_TEXTURE0+shadowTextureOffset);
		glBindTexture(GL_TEXTURE_2D, finalShadowTexture);
	}

	void blurShadows() {
		static Shader pingPongShader("PingPong.fs", "PingPong.vs");
		static unsigned int pingpongFBO[2] = { 0,0 };
		static unsigned int pingpongBuffer[2] = { 0,0 };
		if (pingpongFBO[0] == 0) {
			glGenFramebuffers(2, pingpongFBO);
			glGenTextures(2, pingpongBuffer);
			for (unsigned int i = 0; i < 2; i++)
			{
				glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[i]);
					glBindTexture(GL_TEXTURE_2D, pingpongBuffer[i]);
					glTexImage2D(GL_TEXTURE_2D, 0, GL_RG32F, shadowTextureWidth, shadowTextureWidth, 0, GL_RG, GL_FLOAT, NULL);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
					float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
					glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
					glFramebufferTexture2D(	GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pingpongBuffer[i], 0);
				glBindFramebuffer(GL_FRAMEBUFFER, 0);
			}
		}

		bool horizontal = true, first_iteration = true;
		int amount = 4;
		pingPongShader.use();
		for (unsigned int i = 0; i < amount; i++)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[horizontal]);
				pingPongShader.setInt("horizontal", horizontal);
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, first_iteration ? depthMapTexture : pingpongBuffer[!horizontal]);
		

				OpenGLCommon::drawQuad();
				horizontal = !horizontal;
				if (first_iteration) first_iteration = false;
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		finalShadowTexture = pingpongBuffer[!horizontal];


	}

	~DirectionalLight() {
		glDeleteFramebuffers(1,&depthMapFBO);
		glDeleteTextures(1,&depthMapTexture);
		glDeleteTextures(1, &depthMap);
	}
	
	//void calculateDirectionalShadows(Sj)
};

class PointLight : public Light {
private:
	glm::vec3 position;
	double radius =100;

	const unsigned shadowTextureOffset = 2;
	unsigned int shadowCubeMapFBO=0,shadowCubeMap = 0;
	void createShadowCubeMap() {
		glGenFramebuffers(1, &shadowCubeMapFBO);
		glGenTextures(1, &shadowCubeMap);
		glBindTexture(GL_TEXTURE_CUBE_MAP, shadowCubeMap);
		for (unsigned int i = 0; i < 6; ++i) {
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT,
				shadowTextureWidth, shadowTextureWidth, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
		}
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		
		glBindFramebuffer(GL_FRAMEBUFFER, shadowCubeMapFBO);
			glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, shadowCubeMap, 0);
			glDrawBuffer(GL_NONE);
			glReadBuffer(GL_NONE);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
public:
	PointLight(glm::vec3 Position = glm::vec3(0), glm::vec3 Ambient = glm::vec3(0.2), glm::vec3 Diffuse = glm::vec3(0.5),
		glm::vec3 Specular = glm::vec3(1.0), double Radius = 15) : Light(Ambient,Diffuse,Specular),position(Position) , radius(Radius) {

		shadowTextureWidth = 1024;
	}

	void bindShadowBuffer() {
		if (shadowCubeMapFBO == 0) {
			createShadowCubeMap();
		}
		glViewport(0, 0, shadowTextureWidth, shadowTextureWidth);
		glBindFramebuffer(GL_FRAMEBUFFER, shadowCubeMapFBO);
		glClear(GL_DEPTH_BUFFER_BIT);
	}

	void bindShadowTexture(int id) {
		glActiveTexture(GL_TEXTURE0 +shadowTextureOffset+id);
		glBindTexture(GL_TEXTURE_CUBE_MAP, shadowCubeMap);
	}
	void setPosition(const glm::vec3& pos) {
		position = pos;
	}

	double getRadius() {
		return radius;
	}
	void setShadowMatrices(Shader& shader) {
		glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, float(radius));

		shader.use();
		static const std::pair<glm::vec3, glm::vec3> frontAndUpTable[] = { {{1,0,0},{0,-1,0}},
																		 {{-1,0,0},{0,-1,0}},
																		 {{0,1,0},{0,0,1}},
																		 {{0,-1,0},{0,0,-1}},
																		 {{0,0,1},{0,-1,0}},
																		 {{0,0,-1},{0,-1,0}}};

		glm::mat4 shadowTransforms[6];
		for (int i = 0; i < 6; i++) {
			shadowTransforms[i] = shadowProj * glm::lookAt(position, position + frontAndUpTable[i].first, frontAndUpTable[i].second);
		}
		for (unsigned int i = 0; i < 6; ++i)		{
			shader.setMat4("shadowMatrices[" + std::to_string(i) + "]", shadowTransforms[i]);
		}

		shader.setVec3("lightPos", position);
		shader.setFloat("far_plane", radius);



	}

	glm::vec3 getPosition() {
		return position;
	}

	void setShaderUniforms(Shader& shader,int id) {
		shader.use();
		shader.setInt("pointLight[" + std::to_string(id)+ "].shadowTexture", shadowTextureOffset + id);
		shader.setVec3("pointLight[" + std::to_string(id) + "].position", position);

		shader.setVec3("pointLight[" + std::to_string(id) + "].ambient", ambientColor);
		shader.setVec3("pointLight[" + std::to_string(id) + "].diffuse", diffuseColor);
		shader.setVec3("pointLight[" + std::to_string(id) + "].specular", specularColor);
		shader.setFloat("pointLight[" + std::to_string(id) + "].specularExponent", specularExponent);


		shader.setFloat("pointLight[" + std::to_string(id) + "].farPlane", radius);

	
	}



};
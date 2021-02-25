#pragma once
#include <glm/glm.hpp>
#include "Shader.h"
class GlobaLighting {
private:
	const unsigned shadowTextureOffset = 1;
	const unsigned int SHADOW_WIDTH = 4*1024;
	const double shadowWorldWidth = 500;
	const glm::mat4 shadowProjection = glm::ortho(-shadowWorldWidth, shadowWorldWidth, -shadowWorldWidth, shadowWorldWidth, 1.0, shadowWorldWidth*5);
	
	glm::vec3 ambientColor = glm::vec3(0.5);
	glm::vec3 diffuseColor = glm::vec3(1);

	glm::vec3 specularColor = glm::vec3(0.5);
	float specularExponent = 32;

	glm::vec3 globalLightDir = glm::vec3(0.1, 0.5, 0.1);

	unsigned int depthMap = 0, depthMapFBO = 0, depthMapTest = 0;

	void createShadowTexture(){

		glGenFramebuffers(1, &depthMapFBO);

		glGenTextures(1, &depthMap);
		glBindTexture(GL_TEXTURE_2D, depthMap);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RG32F,
			SHADOW_WIDTH, SHADOW_WIDTH, 0, GL_RG, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

		glGenTextures(1, &depthMapTest);
		glBindTexture(GL_TEXTURE_2D, depthMapTest);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
			SHADOW_WIDTH, SHADOW_WIDTH, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		//float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

		
		glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, depthMap, 0);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMapTest, 0);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		
	}
public:
	GlobaLighting() {}
	GlobaLighting(const glm::vec3& AmbientColor, const glm::vec3& DiffuseColor, const glm::vec3& SpecularColor, float SpecularExponent)
		: ambientColor(AmbientColor), diffuseColor(DiffuseColor), specularColor(SpecularColor), specularExponent(SpecularExponent){}

	void setGlobaLightDirection(const glm::vec3& dir) {
		globalLightDir = dir;
	}

	void setShaderUniforms(Shader& shader) {
		shader.use();
		shader.setVec3("ambientColor", ambientColor);
		shader.setVec3("diffuseColor", diffuseColor);
		shader.setVec3("specularColor", specularColor);

		shader.setFloat("specularExponent", specularExponent);
		shader.setVec3("globalLightDir", globalLightDir);
		shader.setInt("shadowTexture", shadowTextureOffset);
	}

	void bindDirectionalShadowBuffer() {
		if (depthMapFBO == 0) {
			createShadowTexture();
		}
		glViewport(0, 0, SHADOW_WIDTH, SHADOW_WIDTH);
		glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
		glClear(GL_DEPTH_BUFFER_BIT);
	}



	void setDirectionalShadowMatrices(Shader& shader, glm::vec3 cameraPos) {
		shader.use();
		shader.setMat4("shadowProjection",shadowProjection);
		glm::vec3 shadowDir = glm::vec3(1);
		glm::vec3 shadowOrigin = cameraPos + shadowDir*glm::vec3(2*shadowWorldWidth);
		glm::mat4 shadowView = glm::lookAt(shadowOrigin, shadowOrigin-glm::vec3(1,1,1), glm::vec3(0, 1, 0));
		shader.setMat4("shadowView", shadowView);
		
	}

	void bindDirectionalShadowTexture() {
		glActiveTexture(GL_TEXTURE0+shadowTextureOffset);
		glBindTexture(GL_TEXTURE_2D, depthMap);
	}

	~GlobaLighting() {
		glDeleteFramebuffers(1,&depthMapFBO);
		glDeleteTextures(1,&depthMap);
	}
	
	//void calculateDirectionalShadows(Sj)
};
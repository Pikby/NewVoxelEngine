#pragma once
#include <glm/glm.hpp>
#include "Shader.h"
class GlobaLighting {
private:
	glm::vec3 ambientColor = glm::vec3(0.5);
	glm::vec3 diffuseColor = glm::vec3(1);

	glm::vec3 specularColor = glm::vec3(0.5);
	float specularExponent = 32;

	glm::vec3 globalLightDir = glm::vec3(0.1, 0.5, 0.1);

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
		shader.setVec3("specularCOde", specularColor);

		shader.setFloat("specularExponent", specularExponent);
		shader.setVec3("globalLightDir", globalLightDir);
	}
};
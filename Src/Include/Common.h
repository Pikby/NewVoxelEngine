#pragma once
#include <glm/glm.hpp>

#include <glad/glad.h> 
#include <GLFW/glfw3.h>
namespace OpenGLCommon {
	void drawQuad();
	

}

template <typename T> int sign(T val) {
	return (T(0) < val) - (val < T(0));
}
#pragma once


#include <glad/glad.h> 
#include <GLFW/glfw3.h>
#include <map>
#include "Camera.h"

enum class Action { MoveForward, MoveBackwards, MoveLeft, MoveRight, PrimaryAction, SecondaryAction};




class InputHandler {
private:
	static std::map<int, int> actions;
	static Camera camera;


public:
	static void notify();
	static void handleAction(Action action);
	static void init(GLFWwindow* window);
	static bool pollKey(int key);

	static void processMouseMovement(GLFWwindow* window, double xpos, double ypos);
	static void keyCallBack(GLFWwindow* window, int key, int scancode, int action, int mode);
	static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
	
	static Camera& getCamera();
	
};
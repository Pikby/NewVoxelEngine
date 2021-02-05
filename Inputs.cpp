

#include <glad/glad.h> 
#include <GLFW/glfw3.h>
#include <map>
#include "Camera.h"
#include "Inputs.h"
#include <iostream>

std::map<int, Action> keyMap = { {GLFW_KEY_W,Action::MoveForward },
								{GLFW_KEY_S,Action::MoveBackwards},
								{GLFW_KEY_A,Action::MoveLeft},
								{GLFW_KEY_D,Action::MoveRight},
								{GLFW_MOUSE_BUTTON_LEFT,Action::PrimaryAction},
								{GLFW_MOUSE_BUTTON_RIGHT,Action::SecondaryAction} };


Camera InputHandler::camera;
std::map<int, int> InputHandler::actions;


void InputHandler::init(GLFWwindow* window) {
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPosCallback(window, InputHandler::processMouseMovement);
	glfwSetKeyCallback(window, InputHandler::keyCallBack);
	glfwSetMouseButtonCallback(window, InputHandler::mouseButtonCallback);
}

void InputHandler::processMouseMovement(GLFWwindow* window, double xpos, double ypos) {
	static float lastX, lastY;
	static bool firstMouse = true;
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}
	//std::cout << xpos << ":" << ypos << "\n";
	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos;

	lastX = xpos;
	lastY = ypos;


	camera.processMouseMovement(xoffset, yoffset);

}

void InputHandler::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
	if (action == GLFW_RELEASE) {
		actions.erase(button);
	}
	else actions.insert({ button, action });
}

void InputHandler::keyCallBack(GLFWwindow* window, int key, int scancode, int action, int mode) {
	
	if (action == GLFW_RELEASE) {
		actions.erase(key);
	}
	else actions.insert({key, action});
	
}

void InputHandler::notify() {
	for (auto action : actions) {
		InputHandler::handleAction(keyMap[action.first]);
	}
}

void InputHandler::handleAction(Action action) {
	switch (action) {
	case(Action::MoveForward):
		camera.processKeyboard(Camera_Movement::FORWARD, 0.1);
		break;
	case(Action::MoveBackwards):
		camera.processKeyboard(Camera_Movement::BACKWARD, 0.1);
		break;
	case(Action::MoveLeft):
		camera.processKeyboard(Camera_Movement::LEFT, 0.1);
		break;
	case(Action::MoveRight):
		camera.processKeyboard(Camera_Movement::RIGHT, 0.1);
		break;
	case(Action::PrimaryAction):
		//std::cout << "left click\n";
		break;
	default:
		break;

	}


}

bool InputHandler::pollKey(int key) {
	return actions.contains(key);
}


Camera& InputHandler::getCamera() {
	return camera;
}

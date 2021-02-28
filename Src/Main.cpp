#include <btBulletDynamicsCommon.h>
#include <glad/glad.h> 
#include <GLFW/glfw3.h>
#include <iostream>

#include "Include/Chunk.h"
#include "Include/World.h"
#include "Include/Inputs.h"

#include "../Include/Camera.h"
#include "../Include/Model.h"
#include "../Include/PlayerCharacter.h"



std::vector<unsigned int> Shader::shaderList;
std::string Shader::filePath = "Shaders/";

class MP3Instance {
public:
	void run() {
		glfwInit();
		makeWindow();
		InputHandler::init(window);
		mainLoop();
		cleanup();
	}

private:

	GLFWwindow* window;
	void makeWindow() {
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

		
		window = glfwCreateWindow(800, 600, "LearnOpenGL", NULL, NULL);
		if (window == NULL) {
			std::cout << "Failed to create GLFW window" << std::endl;
			glfwTerminate();
			throw - 1;
		}
		glfwMakeContextCurrent(window);


		if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
			std::cout << "Failed to initialize GLAD" << std::endl;
			throw - 1;
		}

		glViewport(0, 0, 800, 600);
		glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);


	}

	static void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
		glViewport(0, 0, width, height);
	}



	void mainLoop() {	
		World world;		
		Entity* entity = new Torch(glm::vec3(10, 1, 0));
		world.addEntity(entity);

		Entity* entity2 = new Torch(glm::vec3(-10, 2, 0));
		world.addEntity(entity2);

		Entity* mainChar = new PlayerCharacter(glm::vec3(0, 0, 0));
		world.addEntity(mainChar);

		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		
		glEnable(GL_DEPTH_TEST);
		
		Shader chunkShader("Chunk.fs", "Chunk.vs");
		Shader normShader("Norm.fs", "Norm.gs", "Norm.vs");
		Shader debugShader("DebugDrawer.fs", "DebugDrawer.vs");

		Camera& camera = InputHandler::getCamera();



		btDiscreteDynamicsWorld* physicsWorld = world.getPhysicsWorld();
		while (!glfwWindowShouldClose(window)) {
			int width, height;
			glfwGetWindowSize(window,&width, &height);
			glm::mat4 projection = glm::perspective(glm::radians(45.0f), float(width) / float(height), 0.1f, 1000.0f);

			Shader::setGlobalMat4("globalProjection",projection);
			Shader::setGlobalMat4("globalView", camera.getViewMatrix());

			glViewport(0,0,width, height);

			glfwSwapBuffers(window);
			glfwPollEvents();
			InputHandler::notify();
			world.scanForChunks(camera.getPosition());
			world.update(physicsWorld, camera);

	
			//Early testing function
			if (InputHandler::pollKey(GLFW_MOUSE_BUTTON_LEFT)) {
				entity->setPosition(glm::vec3(camera.getPosition()));
			}
	
			if (InputHandler::pollKey(GLFW_MOUSE_BUTTON_RIGHT)) {
				world.placeVoxel(Empty, camera);
			}


			//glClearColor(154.0/255.0, 203.0/255.0, 1.0, 1.0f);
			glClearColor(0.0, 0.0, 0.0, 1);
			glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
		

			//world.drawDirectionalShadows(camera);
			world.drawPointShadows(camera);
			glViewport(0, 0, width, height);
	
			world.drawDebugHitboxes(debugShader);
			world.drawEntities(chunkShader, camera);
			world.drawWorld(chunkShader,camera);
			world.drawClouds(camera);
	
		}
	}

	void cleanup() {
		glfwTerminate();
	}
};


int main() {
	MP3Instance app;
	try {
		app.run();

	}
	catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
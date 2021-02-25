#include <btBulletDynamicsCommon.h>



#include <glad/glad.h> 
#include <GLFW/glfw3.h>
#include <iostream>

#include "Model.h"
#include "Chunk.h"
#include "World.h"
#include "Camera.h"
#include "Inputs.h"


std::vector<unsigned int> Shader::shaderList;

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


	void drawLoop() {

	}

	void mainLoop() {
		Shader modelShader("Model.fs","Model.vs");

		glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 1000.0f);
		glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 1.0f, -10.0f),
			glm::vec3(0.0f, 0.0f, 0.0f),
			glm::vec3(0.0f, 1.0f, 0.0f));

		//view = camera.GetViewMatrix();
		glm::mat4 modelMat = glm::scale(glm::mat4(1.f), glm::vec3(0.01f));
		modelShader.use();
		modelShader.setMat4("model", modelMat);
		modelShader.setInt("objTexture", 0);
		
		World world;		
		Entity* entity = new Entity(glm::vec3(0, 1, 0), glm::vec3(0, 1, 0));
		world.addEntity(entity);

		Entity* entity2 = new Entity(glm::vec3(0, 2, 0), glm::vec3(0, 1, 0));
		world.addEntity(entity2);

		std::cout << "World rendered\n";
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		
		glEnable(GL_DEPTH_TEST);
		
		Shader chunkShader("Chunk.fs", "Chunk.vs");
		Shader normShader("Norm.fs", "Norm.gs", "Norm.vs");
		Shader cloudShader("Cloud.fs", "Cloud.vs");
		Shader debugShader("DebugDrawer.fs", "DebugDrawer.vs");


		Camera& camera = InputHandler::getCamera();



		btDiscreteDynamicsWorld* physicsWorld = world.getPhysicsWorld();
		while (!glfwWindowShouldClose(window)) {
			int width, height;
			glfwGetWindowSize(window,&width, &height);
			projection = glm::perspective(glm::radians(45.0f), float(width) / float(height), 0.1f, 1000.0f);




			Shader::setGlobalMat4("globalProjection",projection);
			Shader::setGlobalMat4("globalView", camera.getViewMatrix());

			glViewport(0,0,width, height);

			world.scanForChunks(camera.Position);
			glfwSwapBuffers(window);
			glfwPollEvents();
			InputHandler::notify();

			static bool flag = true;

			if (InputHandler::pollKey(GLFW_MOUSE_BUTTON_LEFT)) {
				Entity* entity = new Entity(camera.Position, glm::vec3(0, 1, 0));
				world.addEntity(entity);
				//world.placeVoxel(Temp, camera);
				//flag = false;
			}
			else {
				flag = true;
			}

			if (InputHandler::pollKey(GLFW_MOUSE_BUTTON_RIGHT)) {
	
				world.placeVoxel(Empty, camera);
				//flag = false;
			}
			glClearColor(154.0/255.0, 203.0/255.0, 1.0, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

			world.drawDirectionalShadows(camera);

			glViewport(0, 0, width, height);
			
			world.drawChunks(chunkShader,camera);
			world.drawClouds(cloudShader, camera);

		
			//debugShader.use();
			world.update(physicsWorld);
			//world.drawDebugHitboxes(debugShader);


			world.drawEntities(cloudShader, camera);
			world.drawTranslucentChunks(chunkShader, camera);
			
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
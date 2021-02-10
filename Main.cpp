
#include <glad/glad.h> 
#include <GLFW/glfw3.h>
#include <iostream>

#include "Model.h"
#include "Chunk.h"
#include "World.h"
#include "Camera.h"
#include "Inputs.h"

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
		glm::mat4 modelMat = glm::scale(glm::mat4(1), glm::vec3(0.01));
		modelShader.use();
		modelShader.setMat4("projection", projection);
		modelShader.setMat4("view", view);
		modelShader.setMat4("model", modelMat);
		modelShader.setInt("objTexture", 0);
		Model model("bunny.obj");

		World world;		
		const int renderDistance = 10;
		for (int x = -renderDistance; x < renderDistance; x++) {
			for (int y = -renderDistance; y < renderDistance; y++) {
				for (int z = -renderDistance; z < renderDistance; z++) {
					glm::ivec3 curPos = glm::ivec3(x, y, z);
					world.loadChunk(glm::ivec3(x, y, z));
				}
			}
		}
		
		std::cout << "World rendered\n";
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		
		glEnable(GL_DEPTH_TEST);
		
		Shader chunkShader("Chunk.fs", "Chunk.vs");
		chunkShader.use();
		chunkShader.setMat4("projection", projection);

		Shader normShader("Norm.fs", "Norm.gs", "Norm.vs");
		normShader.use();
		normShader.setMat4("projection", projection);

		Camera& camera = InputHandler::getCamera();
		while (!glfwWindowShouldClose(window)) {

			world.scanForChunks(camera.Position);
			glfwSwapBuffers(window);
			glfwPollEvents();
			InputHandler::notify();

			static bool flag = true;

			if (InputHandler::pollKey(GLFW_MOUSE_BUTTON_LEFT)) {
				world.placeVoxel(Full, camera);
				//flag = false;
			}
			else {
				flag = true;
			}

			if (InputHandler::pollKey(GLFW_MOUSE_BUTTON_RIGHT)) {
				world.placeVoxel(Empty, camera);
				//flag = false;
			}
			glClearColor(1, 0.19, 0.34, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
			world.drawChunks(chunkShader,camera);
			
			/*
			normShader.use();
			normShader.setMat4("view", view);
			world.drawChunks(normShader);
			*/
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
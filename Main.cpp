
#include <glad/glad.h> 
#include <GLFW/glfw3.h>
#include <iostream>

#include "Model.h"
#include "Chunk.h"
#include "Camera.h"

Camera camera(glm::vec3(0.0f, 0.0f, -10.0f));
void processMouseMovment(GLFWwindow* window, double xpos, double ypos) {
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

	camera.ProcessMouseMovement(xoffset, yoffset);

}

void keyCallBack(GLFWwindow* window, int key, int scancode, int action, int mode)
{

	if (key == GLFW_KEY_W) {
		camera.ProcessKeyboard(FORWARD, 0.1);
	}
	else if (key == GLFW_KEY_A) {
		camera.ProcessKeyboard(LEFT, 0.1);
	}
	else if (key == GLFW_KEY_S) {
		camera.ProcessKeyboard(BACKWARD, 0.1);
	}
	else if (key == GLFW_KEY_D) {
		camera.ProcessKeyboard(RIGHT, 0.1);
	}


}

class MP3Instance {
public:
	void run() {
		glfwInit();
		makeWindow();
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
		if (window == NULL)
		{
			std::cout << "Failed to create GLFW window" << std::endl;
			glfwTerminate();
			throw - 1;
		}
		glfwMakeContextCurrent(window);


		if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
		{
			std::cout << "Failed to initialize GLAD" << std::endl;
			throw - 1;
		}

		glViewport(0, 0, 800, 600);
		glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		glfwSetCursorPosCallback(window, processMouseMovment);
		glfwSetKeyCallback(window, keyCallBack);
	}

	static void framebuffer_size_callback(GLFWwindow* window, int width, int height)
	{
		glViewport(0, 0, width, height);
	}

	void mainLoop() {
		Shader modelShader("Model.fs","Model.vs");
	


		glm::mat4 projection = glm::perspective(glm::radians(45.0f), 1920.0f / 1080.0f, 0.1f, 100.0f);
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
		Chunk chunk1;
		chunk1.mesh();
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		
		glEnable(GL_DEPTH_TEST);
		
		Shader chunkShader("Chunk.fs", "Chunk.vs");
		chunkShader.use();
		chunkShader.setMat4("projection", projection);

		Shader normShader("Norm.fs", "Norm.gs", "Norm.vs");
		normShader.use();
		normShader.setMat4("projection", projection);
		while (!glfwWindowShouldClose(window))
		{
			glfwSwapBuffers(window);
			glfwPollEvents();

			//std::cout << "Loop\n";
			glClearColor(1, 0.19, 0.34, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
			modelShader.use();
			view = camera.GetViewMatrix();
			//std::cout << glm::to_string(view) << "\n";
			modelShader.setMat4("view", view);
			//model.Draw(modelShader);


			chunkShader.use();
			chunkShader.setMat4("view", view);
			chunk1.draw(chunkShader);

			normShader.use();
			normShader.setMat4("view", view);
			chunk1.draw(normShader);
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
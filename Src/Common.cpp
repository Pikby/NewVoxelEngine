#include "Include/Common.h"
#include "Include/Shader.h"
void OpenGLCommon::drawQuad(){
	 struct QuadVertex {
		 glm::vec3 pos;
		 glm::vec2 tex;
	 };
	 
	 static QuadVertex quadVertices[] = {
		 // positions          // colors      // texture coords
		 {{1.0f,  1.0, 0.0f},   {1.0f, 1.0f}},   // top right
		 {{1.0f, -1.0f, 0.0f},   {1.0f, 0.0f}},   // bottom right
		 {{-1.0f, -1.0f, 0.0f},  {0.0f, 0.0f}},   // bottom left

		 {{1.0f,  1.0f, 0.0f},   {1.0f, 1.0f}},   // top right
		 {{-1.0f, -1.0f, 0.0f},  {0.0f, 0.0f}},   // bottom left
		 {{-1.0f,  1.0f, 0.0f},  {0.0f, 1.0f}},    // top left 
	 };
	 static unsigned int VAO = 0, VBO = 0;
	 if (VAO == 0) {
		 glGenVertexArrays(1, &VAO);
		 glGenBuffers(1, &VBO);

		 glBindVertexArray(VAO);
		 glBindBuffer(GL_ARRAY_BUFFER, VBO);
		 glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(QuadVertex), quadVertices, GL_STATIC_DRAW);

		 glEnableVertexAttribArray(0);
		 glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(decltype(quadVertices[0])), (void*)(offsetof(QuadVertex, pos)));

		 glEnableVertexAttribArray(1);
		 glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(decltype(quadVertices[0])), (void*)(offsetof(QuadVertex, tex)));
		 glBindVertexArray(0);
	 }
	 glDisable(GL_CULL_FACE);
	 glBindVertexArray(VAO);
	 glDrawArrays(GL_TRIANGLES, 0, 6);
	 glBindVertexArray(0);
	 glEnable(GL_CULL_FACE);
	}


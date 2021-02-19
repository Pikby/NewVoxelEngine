#pragma once
#include <cstdint>
#include <glad/glad.h> 
#include <GLFW/glfw3.h>
class ChunkBufferObject {
private:
	uint32_t VAO = 0;
	uint32_t VBO = 0;
	uint32_t EBO = 0;

	size_t verticeCount;
	size_t indiceCount;

public:
	ChunkBufferObject(size_t VerticeCount, size_t IndiceCount) : verticeCount(VerticeCount), indiceCount(IndiceCount) {
		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, VerticeCount * sizeof(decltype(surfacePoints[0])), &(surfacePoints[0]), GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(decltype(indices[0])), &(indices[0]), GL_STATIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(decltype(surfacePoints[0])), (void*)(offsetof(ChunkVertex, pos)));

		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(decltype(surfacePoints[0])), (void*)(offsetof(ChunkVertex, norm)));

		glEnableVertexAttribArray(2);
		glVertexAttribIPointer(2, 1, GL_INT, sizeof(decltype(surfacePoints[0])), (void*)(offsetof(ChunkVertex, color)));
		glBindVertexArray(0);
	}

}
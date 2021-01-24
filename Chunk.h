#pragma once
#include <cstdint>
#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/detail/compute_vector_relational.hpp>

#include "Shader.h"
#include "Array3D.h"
#include <vector>

struct Voxel {
	uint8_t color;
	int8_t val;
	bool operator==(const Voxel& rhs) {
		return color == rhs.color;
	}

	bool operator!=(const Voxel& rhs) {
		return color != rhs.color;
	}
};

const Voxel Empty{ 0,100 };
const Voxel Full{ 0xff,-100 };

const int ChunkSize = 16;


class Chunk {
private:
	CubeArray<Voxel, ChunkSize> voxelArray;
	uint32_t VAO = 0;
	uint32_t VBO = 0;
	uint32_t EBO = 0;
	long int verticesCount = 0;
	long int indicesCount = 0;
public:
	void mesh();

	void draw(Shader& shader);
	Chunk();
	Chunk& operator=(const Chunk&) = delete;
	Chunk(const Chunk&) = delete;

};
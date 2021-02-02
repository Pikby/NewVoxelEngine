#pragma once
#include <cstdint>
#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/detail/compute_vector_relational.hpp>
#include <vector>
#include <unordered_map>
#include <future>
#include "Shader.h"
#include "Array3D.h"



enum class ChunkFlags{Built,Meshed,Loaded,Empty};

struct Voxel {
	uint16_t color;
	int8_t val;
	bool operator==(Voxel& rhs) {
		return color == rhs.color && val == rhs.val;
	}

	bool operator!=(Voxel& rhs) {
		return !(this->operator==(rhs));
	}
};

struct ChunkVertex {
	glm::vec3 pos;
	glm::vec3 norm;
	uint16_t color;
};

struct ChunkCube {
	int64_t indice;
	Voxel voxels[8];
};

struct ChunkMesh {
	std::vector<ChunkVertex> surfacePoints;
	std::vector<glm::ivec3> indices;

};
static Voxel Empty{ 0xf0f0,100 };
static Voxel Full{ 0xf0f0,-100 };
static Voxel Error{ 0,0 };

static const glm::ivec3 chunkNeighboursTable[7] = { {1,1,1},{0,1,1},{1,0,1},{1,1,0},{1,0,0},{0,1,0},{0,0,1}, };
const int ChunkSize = 16;


class Chunk {
private:
	CubeArray<Voxel, ChunkSize> voxelArray;
	const glm::ivec3 chunkPos;
	uint32_t VAO = 0;
	uint32_t VBO = 0;
	uint32_t EBO = 0;
	size_t verticesCount = 0;
	size_t indicesCount = 0;

	ChunkFlags chunkFlag = ChunkFlags::Empty;
	std::weak_ptr<Chunk> chunkNeighbours[7];
	std::future<std::shared_ptr<ChunkMesh>> meshTask;
	

	std::shared_ptr<ChunkMesh> calculateMesh();
	void buildBufferObjects(std::vector<ChunkVertex>& surfacePoints, std::vector<glm::ivec3>& indices);
public:
	void mesh();
	void setNeighbours(const glm::ivec3& pos, std::unordered_map<glm::ivec3, std::shared_ptr<Chunk>>& chunks);
	Voxel& getVoxel(const glm::ivec3& po);

	void draw(Shader& shader);
	Chunk(const glm::vec3& pos);
	Chunk() = delete;
	~Chunk();
	Chunk& operator=(const Chunk&) = delete;
	Chunk(const Chunk&) = delete;

};
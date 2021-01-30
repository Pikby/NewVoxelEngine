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



enum class ChunkFlags{Built,Loaded,Empty};

struct Voxel {
	uint8_t color;
	int8_t val;
	bool operator==(Voxel& rhs) {
		return color == rhs.color && val == rhs.val;
	}

	bool operator!=(Voxel& rhs) {
		return !(this->operator==(rhs));
	}
};

static Voxel Empty{ 0,100 };
static Voxel Full{ 0xff,-100 };
static Voxel Error{ 0,0 };

const int ChunkSize = 16;


class Chunk {
private:
	CubeArray<Voxel, ChunkSize> voxelArray;
	const glm::ivec3 chunkPos;
	uint32_t VAO = 0;
	uint32_t VBO = 0;
	uint32_t EBO = 0;
	long int verticesCount = 0;
	long int indicesCount = 0;

	ChunkFlags chunkFlag = ChunkFlags::Empty;
	std::weak_ptr<Chunk> chunkNeighbours[7];
	std::future<void> meshTask;
	//static const glm::ivec3 chunkNeighboursTable[6] =  { {1,0,0},{-1,0,0},{0,1,0},{0,-1,0},{0,0,1},{0,0,-1} };
	void calculateMesh();
public:
	void mesh();
	void setNeighbours(const glm::ivec3& pos, std::unordered_map<glm::ivec3, std::shared_ptr<Chunk>>& chunks);
	Voxel& getVoxel(const glm::ivec3& po);

	void draw(Shader& shader);
	Chunk(const glm::vec3& pos);
	~Chunk();
	Chunk& operator=(const Chunk&) = delete;
	Chunk(const Chunk&) = delete;

};
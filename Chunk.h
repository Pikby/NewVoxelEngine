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
#include "Color.h"
#include "Voxel.h"

enum class ChunkFlags{LoadedInRAM,QueuedToMesh,LoadedToGPU,Empty};

enum class VoxelRenderStyle{Opaque,Translucent};

struct ChunkVertex {
	glm::vec3 pos;
	glm::vec3 norm;
	uint32_t color;
};

struct ChunkCube {
	int64_t indice;
	VoxelKey voxels[8];
};

struct ChunkMesh {
	std::vector<ChunkVertex> surfacePoints;
	std::vector<glm::ivec3> indices;

};

struct BufferObject {
	uint32_t VAO = 0;
	uint32_t VBO = 0;
	uint32_t EBO = 0;
	size_t verticesCount = 0;
	size_t indicesCount = 0;

	void deleteBuffers() {
		glDeleteVertexArrays(1, &VAO);
		glDeleteBuffers(1, &VBO);
		glDeleteBuffers(1, &EBO);
	}

	void drawBuffer() {
		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, indicesCount, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
};


//Table for the 7 chunk neighbours in the postive x,y,z directions respectively
static const glm::ivec3 chunkNeighboursTable[7] = { {1,1,1},{0,1,1},{1,0,1},{1,1,0},{1,0,0},{0,1,0},{0,0,1}, };

//Table for the 8 corners of a cube
static const glm::ivec3 cubeCorners[8] = { {0,0,0},{1,0,0},{0,1,0},{1,1,0}, {0,0,1},{1,0,1},{0,1,1},{1,1,1} };

//Table for the 12 edges of a cube with respect to the corners
static const std::pair<int, int> cubeEdges[12] = { {0,1}, {0,2}, {1,3}, {2,3}, {4,5}, {4,6}, {5,7}, {6,7}, {0,4}, {1,5}, {2,6}, {3,7} };

const int ChunkSize = 32;


class Chunk {
private:
	CubeArray<VoxelKey, ChunkSize> voxelArray;
	const glm::ivec3 chunkPos;
	BufferObject opaqueBuffer;
	BufferObject translucentBuffer;

	std::weak_ptr<Chunk> chunkNeighbours[7];

	std::future< std::pair< std::shared_ptr<ChunkMesh>, std::shared_ptr<ChunkMesh>>> meshTask;
	std::shared_future<void> buildTask;


	auto getAllRelevantVoxels(const std::array<std::shared_ptr<Chunk>, 7> chunkNeighbours);

	std::pair< std::shared_ptr<ChunkMesh>, std::shared_ptr<ChunkMesh>> getAllMeshes(std::array<std::shared_ptr<Chunk>, 7> chunkNeighbours);
	//Takes the current chunk and all the surrounding voxels and returns a list of vertices and indices
	std::shared_ptr<ChunkMesh> calculateMesh(std::shared_ptr<CubeArray<VoxelKey, ChunkSize + 2>> fullVoxels, VoxelRenderStyle style);

	//Takes a list of vertices and indices and creates buffer objects for them
	BufferObject buildBufferObject(std::vector<ChunkVertex>& surfacePoints, std::vector<glm::ivec3>& indices);
	void deleteBufferObjects();
	std::atomic<ChunkFlags> chunkFlag = ChunkFlags::Empty;
public:
	//Creates an async job to mesh the given chunk
	void mesh();

	//All neighbours should exist in the chunktable before calling this function, function finds all surrouding chunks and loads them
	void setNeighbours(const glm::ivec3& pos, std::unordered_map<glm::ivec3, std::shared_ptr<Chunk>>& chunks);
	VoxelKey& getVoxel(const glm::ivec3& po);

	//Draws the given chunk with a shader, if the mesh is not valid/built function will return, otherwise it will automatically build the buffers
	void draw(Shader& shader);

	void drawTranslucent(Shader& shader);
	//fills out VoxelArray using randomly generated algorithm
	void generateChunk();

	bool hasMesh();
	
	void update();

	bool safeToDelete();
	Chunk(const glm::vec3& pos);
	~Chunk();

	const glm::ivec3& getChunkPos(){
		return chunkPos;
	}

	Chunk() = delete;
	Chunk& operator=(const Chunk&) = delete;
	Chunk(const Chunk&) = delete;

};
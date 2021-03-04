#pragma once
#include <cstdint>
#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/detail/compute_vector_relational.hpp>
#include <vector>
#include <unordered_map>
#include <future>
#include <btBulletDynamicsCommon.h>
#include <memory>

#include "../../Include/Shader.h"
#include "../../Include/Array3D.h"
#include "../../Include/Color.h"
#include "../../Include/Voxel.h"

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

class ChunkCollisionObject{
private:
	std::unique_ptr<btTriangleIndexVertexArray> triMesh;
	std::unique_ptr<btTriangleMeshShape> bvhMesh;
	std::unique_ptr<btDefaultMotionState> motion;
	std::unique_ptr<btRigidBody> meshBody;
public:
	ChunkCollisionObject() {}

	void init(btTriangleIndexVertexArray* TriMesh, const glm::ivec3 pos) {
		triMesh = std::unique_ptr<btTriangleIndexVertexArray>(TriMesh);
		bvhMesh = std::make_unique<btBvhTriangleMeshShape>(triMesh.get(), false);

		glm::mat4 model = glm::translate(glm::mat4(1), glm::vec3(pos * ChunkSize));

		btTransform transform;
		transform.setFromOpenGLMatrix((float*)&model);

		motion = std::make_unique<btDefaultMotionState>(transform);
		btRigidBody::btRigidBodyConstructionInfo rbInfo(0.0f, motion.get(), bvhMesh.get(), btVector3(0, 0, 0));
		meshBody = std::make_unique<btRigidBody>(rbInfo);
		meshBody->setFriction(0.5);
		meshBody->setHitFraction(1);
		meshBody->setRestitution(0.1f);
	}
	
	btRigidBody* getRigidBody() {
		return meshBody.get();
	}

};

class Chunk {
private:
	CubeArray<VoxelKey, ChunkSize> voxelArray;
	const glm::ivec3 chunkPos;
	BufferObject opaqueBuffer;
	BufferObject translucentBuffer;

	ChunkCollisionObject collisionObject;



	std::weak_ptr<Chunk> chunkNeighbours[7];

	std::future< std::pair< std::shared_ptr<ChunkMesh>, std::shared_ptr<ChunkMesh>>> meshTask;
	std::shared_future<void> buildTask;

	std::shared_ptr<ChunkMesh> chunkMesh;
	std::shared_ptr<btTriangleIndexVertexArray> collisionMesh;

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
	VoxelKey& getVoxel(const glm::ivec3& pos);

	//Draws the given chunk with a shader, if the mesh is not valid/built function will return, otherwise it will automatically build the buffers
	void draw(Shader& shader);

	//Draws the translucent part of the chunk only, done at the end of the draw calls
	void drawTranslucent(Shader& shader);
	//fills out VoxelArray using randomly generated algorithm
	void generateChunk();

	bool hasMesh();
	

	bool isLoaded();

	void update();

	btRigidBody* const getPhysicsBody();

	//Checks if all chunk threads are finished and the chunk is safe to delete
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
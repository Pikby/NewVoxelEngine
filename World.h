#pragma once

#include <map>
#include <unordered_map>
#include <memory>
#include <optional>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include "Chunk.h"
#include "Camera.h"
#include "GlobalLighting.h"
class World {
private:
	std::unordered_map<glm::ivec3, std::shared_ptr<Chunk>> chunks;
	GlobaLighting globalLighting;

	//Takes block in world position and tests to see if the block change effects any neighbouring chunks, if it does reload them
	void loadNeighbouringChunks(glm::ivec3 pos);
	std::optional<glm::ivec3> findLookingVoxel(Camera& camera);
	inline glm::ivec3 getChunkPos(const glm::vec3& pos);
	inline glm::ivec3 getLocalPos(const glm::vec3& pos);
public:
	
	//Makes sure all surrouding chunks are loaded, then requests the chunk to be meshed
	void loadChunk(const glm::ivec3& pos);

	//Draws all loaded chunks in the world
	void drawChunks(Shader& shader,const Camera& camera);

	//Scans for chunks around position and unloads them 
	void scanForChunks(const glm::vec3& pos);

	//Returns a reference to a voxel in the world
	VoxelKey& getVoxel(const glm::ivec3& pos);

	//Takes a camera argument and attempts to place a voxel wherever the camera is looking
	void placeVoxel(VoxelKey vox, Camera& camera);
};
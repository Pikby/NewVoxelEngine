#pragma once
#include "Chunk.h"
#include "Camera.h"
#include <map>
#include <unordered_map>
#include <memory>
#include <optional>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>
class World {
private:
	std::unordered_map<glm::ivec3, std::shared_ptr<Chunk>> chunks;
public:
	void loadChunk(const glm::ivec3& pos);
	void drawChunks(Shader& shader);

	void loadNeighbouringChunks(glm::ivec3 pos);
	std::optional<glm::ivec3> findLookingVoxel(Camera& camera);
	Voxel& getVoxel(const glm::ivec3& pos);
	void placeVoxel(Voxel vox, Camera& camera);
};
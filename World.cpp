#include <glm/gtc/integer.hpp>
#include <map>
#include <unordered_map>
#include <memory>
#include <future>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include "World.h"
#include "Chunk.h"
#include "Camera.h"



#include <optional>

void World::loadChunk(const glm::ivec3& pos) {
	static const glm::ivec3 chunkNeighboursTable[7] = { {1,1,1},{0,1,1},{1,0,1},{1,1,0},{1,0,0},{0,1,0},{0,0,1}, };
	if (!chunks.contains(pos)) {
		chunks.insert({ pos, std::make_shared<Chunk>(pos) });
	}

	for (int i = 0; i < 7; i++) {
		glm::ivec3 offset = chunkNeighboursTable[i] + pos;
		if (!chunks.contains(offset)) {
			chunks.insert({ offset, std::make_shared<Chunk>(offset) });
		}
	}
	auto newChunk = chunks.at(pos);
	newChunk->setNeighbours(pos,chunks);
	newChunk->mesh();
}

void World::drawChunks(Shader& shader) {
	for (auto chunk : chunks) {
		chunk.second->draw(shader);
	}

}

Voxel& World::getVoxel(const glm::ivec3& pos) {
	const glm::ivec3 chunkPos = glm::floor(glm::vec3(pos) / glm::vec3(ChunkSize));
	const glm::ivec3 localPos = glm::mod(glm::vec3(pos),glm::vec3(ChunkSize));
	//std::cout << glm::to_string(pos) << ":" << glm::to_string(chunkPos) << ":" << glm::to_string(localPos) << "\n";

	
	if (!chunks.contains(chunkPos)) {
		loadChunk(chunkPos);
	}

	Voxel& vox = chunks[chunkPos]->getVoxel(localPos);
	return vox;
}


std::optional<glm::ivec3> World::findLookingVoxel(Camera& camera) {
	glm::vec3 front = camera.Front;
	glm::vec3 pos = camera.Position;

	const float stepSize = 0.2;
	const int maxSteps = 10000;

	for (int i = 0; i < maxSteps; i++) {
		glm::ivec3 voxPos = (pos + front * (i * stepSize));
		Voxel& vox = getVoxel(voxPos);
		if (vox.val < 0)
		{
			//std::cout << "Found " << glm::to_string(glm::ivec3(pos + front * (i * stepSize)));
			return voxPos;
		
		}

	}


	return {};
}

void World::placeVoxel(Voxel vox,Camera& camera) {
	glm::vec3 front = camera.Front;
	glm::vec3 pos = camera.Position;

	const float stepSize = 0.1;
	const int maxSteps = 100;

	for (int i = 0; i < maxSteps; i++) {
		glm::vec3 voxPos = (pos + front * (i * stepSize));
		Voxel& lookingVox = getVoxel(glm::floor(voxPos));
		if (lookingVox.val < 0) {

			if (vox.val > 0) {
				lookingVox = vox;
				loadNeighbouringChunks(voxPos);
			}
			else {
				glm::vec3 newPos = voxPos - front * stepSize;
				Voxel& nearVox = getVoxel(glm::floor(newPos));
				nearVox = vox;

				loadNeighbouringChunks(newPos);
			}
			return;
		}

	}
}



void World::loadNeighbouringChunks(glm::ivec3 pos) {
	glm::ivec3 chunkPos = glm::floor(glm::vec3(pos) / glm::vec3(ChunkSize));
	glm::ivec3 localPos = glm::mod(glm::vec3(pos), glm::vec3(ChunkSize));

	static const glm::ivec3 checks[7] = { {1,1,1},{0,1,1},{1,0,1},{1,1,0},{1,0,0},{0,1,0},{0,0,1}, };
	for (auto check : checks) {
		if (glm::all(glm::lessThanEqual(check-glm::ivec3(1),localPos))) {


			loadChunk(chunkPos - check);
		
		}

	}
	
	loadChunk(chunkPos);
}


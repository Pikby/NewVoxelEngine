#pragma once

#include <map>
#include <unordered_map>
#include <memory>
#include <optional>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>


#include "Entity.h"
#include "Chunk.h"
#include "Camera.h"
#include "GlobalLighting.h"
#include "DebugDrawer.h"
struct PhysicsWorld {
	std::unique_ptr<DebugDrawer> debugDrawer;
	std::unique_ptr<btDefaultCollisionConfiguration> collisionConfiguration;
	std::unique_ptr<btCollisionDispatcher> dispatcher;
	std::unique_ptr<btBroadphaseInterface> overlappingPairCache;
	std::unique_ptr<btSequentialImpulseConstraintSolver> solver;
	std::unique_ptr<btDiscreteDynamicsWorld> physicsWorld;
};

class World {
private:
	std::unordered_map<glm::ivec3, std::shared_ptr<Chunk>> chunks;
	GlobaLighting globalLighting;
	PhysicsWorld physicsWorld;
	std::vector<std::unique_ptr<Entity>> entityList;


	//Takes block in world position and tests to see if the block change effects any neighbouring chunks, if it does reload them
	void loadNeighbouringChunks(glm::ivec3 pos);
	std::optional<glm::ivec3> findLookingVoxel(Camera& camera);
	inline glm::ivec3 getChunkPos(const glm::vec3& pos);
	inline glm::ivec3 getLocalPos(const glm::vec3& pos);

	unsigned int generateTextureFromNoise(const std::string& noiseString);

	void drawQuad();
public:
	World();

	btDiscreteDynamicsWorld* getPhysicsWorld();

	//Makes sure all surrouding chunks are loaded, then requests the chunk to be meshed
	void loadChunk(const glm::ivec3& pos);

	//Draws all loaded chunks in the world
	void drawChunks(Shader& shader,const Camera& camera);
	void drawTranslucentChunks(Shader& shader, const Camera& camera);

	void drawClouds(Shader& shader, const Camera& camera);
	
	void drawEntities(Shader& shader, const Camera& camera);

	void drawDebugHitboxes(Shader& shader) {
		physicsWorld.debugDrawer->draw(shader);
	}

	void drawDirectionalShadows(const Camera& camera);

	//Scans for chunks around position and unloads them 
	void scanForChunks(const glm::vec3& pos);

	void update(btDiscreteDynamicsWorld* physicsWorld);

	//Returns a reference to a voxel in the world
	VoxelKey& getVoxel(const glm::ivec3& pos);

	//Takes a camera argument and attempts to place a voxel wherever the camera is looking
	void placeVoxel(VoxelKey vox, Camera& camera);

	void addEntity(Entity* entity);
};
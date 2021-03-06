#pragma once

#include <map>
#include <unordered_map>
#include <memory>
#include <optional>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>


#include "../../Include/Entity.h"
#include "Chunk.h"
#include "../../Include/Camera.h"
#include "../../Include/Lighting.h"
#include "../../Include/DebugDrawer.h"
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
	std::vector<std::unique_ptr<Entity>> entityList;
	std::vector<PointLight*> pointLightList;

	DirectionalLight globalLighting;
	PhysicsWorld physicsWorld;

	float renderDistance = 10;



	bool drawDebugHitBoxes = false;
	//Takes block in world position and tests to see if the block change effects any neighbouring chunks, if it does reload them
	void loadNeighbouringChunks(const glm::ivec3& pos);
	inline glm::ivec3 getChunkPos(const glm::vec3& pos);
	inline glm::ivec3 getLocalPos(const glm::vec3& pos);

	static unsigned int generateTextureFromNoise(const std::string& noiseString);

	void updateSurroundingVoxels(const glm::ivec3& pos);
public:
	World();

	btDiscreteDynamicsWorld* getPhysicsWorld();

	//Makes sure all surrouding chunks are loaded, then requests the chunk to be meshed
	void loadChunk(const glm::ivec3& pos);

	//Draws all loaded chunks in the world
	void drawChunks(Shader& shader, const Camera& camera, unsigned int renderDistance, bool translucent = false);

	void drawClouds(const Camera& camera);
	
	void drawEntities( const Camera& camera);

	void drawDebugHitboxes(Shader& shader) {
		physicsWorld.debugDrawer->draw(shader);
	}
	std::shared_ptr<Chunk> getChunk(const glm::ivec3& chunkPos);

	void drawDirectionalShadows(const Camera& camera);

	void drawPointShadows(const Camera& camera);
	//Scans for chunks around position and unloads them 
	void scanForChunks(const glm::vec3& pos);

	VoxelKey& getAndUpdateVoxel(const glm::ivec3& pos);

	void drawWorld(Shader& shader, const Camera& camera);

	void update(btDiscreteDynamicsWorld* physicsWorld, const Camera& camera);

	//Returns a reference to a voxel in the world
	VoxelKey& getVoxel(const glm::ivec3& pos);

	//Takes a camera argument and attempts to place a voxel wherever the camera is looking
	void placeVoxel(VoxelKey vox,const Camera& camera);

	void addEntity(Entity* entity);

	void setDebugHitBoxes(bool b) { drawDebugHitBoxes = b; }

	void generateTree(const glm::ivec3& pos);

};
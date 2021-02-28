#include <FastNoise/FastNoise.h>

#include <glm/gtc/integer.hpp>
#include <map>
#include <unordered_map>
#include <memory>
#include <future>
#include <array>
#include <optional>
#include <set>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include "Include/World.h"
#include "Include/Chunk.h"

#include "../Include/Common.h"
#include "../Include/Array3D.h"




inline glm::ivec3 World::getChunkPos(const glm::vec3& pos) {
	return glm::floor(glm::vec3(pos) / glm::vec3(ChunkSize));
}
inline glm::ivec3 World::getLocalPos(const glm::vec3& pos) {
	return glm::mod(glm::vec3(pos), glm::vec3(ChunkSize));
}



World::World() :globalLighting(glm::vec3(0.1,0.1,0.15),glm::vec3(0.1),glm::vec3(0.5)) {

	physicsWorld.debugDrawer = std::make_unique<DebugDrawer>();
	physicsWorld.collisionConfiguration = std::make_unique<btDefaultCollisionConfiguration>();
	physicsWorld.dispatcher = std::make_unique<btCollisionDispatcher>(physicsWorld.collisionConfiguration.get());
	physicsWorld.overlappingPairCache = std::make_unique<btDbvtBroadphase>();
	physicsWorld.solver = std::make_unique<btSequentialImpulseConstraintSolver>();
	physicsWorld.physicsWorld = std::make_unique<btDiscreteDynamicsWorld>(physicsWorld.dispatcher.get(), physicsWorld.overlappingPairCache.get(), physicsWorld.solver.get(), physicsWorld.collisionConfiguration.get());
	
	
	physicsWorld.physicsWorld->setDebugDrawer(physicsWorld.debugDrawer.get());
	physicsWorld.physicsWorld->setGravity(btVector3(0, -10, 0));
}



void World::loadChunk(const glm::ivec3& pos) {
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


void World::drawWorld(Shader& shader, const Camera& camera) {
	shader.use();
	double time = glfwGetTime();

	shader.setFloat("time", time);
	shader.setInt("translucent", 0);
	shader.setVec3("cameraPos", camera.getPosition());

	globalLighting.setShaderUniforms(shader);
	globalLighting.setDirectionalShadowMatrices(shader, camera.getPosition());
	globalLighting.bindShadowTexture();

	/*
	for (int i = 0; i < pointLightList.size(); i++) {
		pointLightList[i]->setShaderUniforms(shader, i);
		pointLightList[i]->bindShadowTexture(i);
	}
	*/

	pointLightList[0]->setShaderUniforms(shader, 0);
	pointLightList[0]->bindShadowTexture(0);

	pointLightList[1]->setShaderUniforms(shader, 1);
	pointLightList[1]->bindShadowTexture(1);
	
	//shader.setInt("pointLightCount", std::min(int(pointLightList.size()), 10));


	pointLightList.clear();
	//light.setShaderUniforms(shader);
	//light.bindShadowTexture();

	drawChunks(shader,camera.getPosition(),20);
	drawTranslucentChunks(shader, camera);
}

void World::drawChunks(Shader& shader, const glm::vec3& origin, unsigned int renderDistance) {

	for (auto& chunk : chunks ) {
		glm::vec3 realPos = chunk.second->getChunkPos() * ChunkSize;
		if (glm::distance(origin, realPos) < renderDistance*ChunkSize) {
			chunk.second->draw(shader);
		}
		
	}

}
void World::drawTranslucentChunks(Shader& shader,const Camera& camera){
	shader.use();
	shader.setInt("translucent", 1);
	glActiveTexture(GL_TEXTURE0);
	static unsigned int waterTexture = generateTextureFromNoise("FwAAAIC/AACAPwAAAAAAAIA/GQAJAAEDAM3MzD0=");
	glBindTexture(GL_TEXTURE_2D, waterTexture);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_CULL_FACE);
	for (auto& chunk : chunks ) {
		chunk.second->drawTranslucent(shader);
	}
	glEnable(GL_CULL_FACE);
}

void World::scanForChunks(const glm::vec3& pos) {
	const int renderDistance = 12;
	for (int x = -renderDistance; x < renderDistance; x++) {
		for (int y = -renderDistance; y < renderDistance; y++) {
			for (int z = -renderDistance; z < renderDistance; z++) {
				glm::ivec3 curPos = glm::vec3(getChunkPos(pos))+glm::vec3(x, y, z);


				if (!chunks.contains(curPos)) {
					loadChunk(curPos);
					continue;
				}

				auto chunk = chunks.at(curPos);
				if(!(chunk->hasMesh())){
					loadChunk(curPos);
				}
			}
		}
	}

}


VoxelKey& World::getVoxel(const glm::ivec3& pos) {
	const glm::ivec3 chunkPos = getChunkPos(pos);
	const glm::ivec3 localPos = getLocalPos(pos);

	
	if (!chunks.contains(chunkPos)) {
		loadChunk(chunkPos);
	}

	VoxelKey& vox = chunks[chunkPos]->getVoxel(localPos);
	return vox;
}


std::optional<glm::ivec3> World::findLookingVoxel(Camera& camera) {
	glm::vec3 front = camera.getFront();
	glm::vec3 pos = camera.getPosition();

	const float stepSize = 0.2;
	const int maxSteps = 10000;

	for (int i = 0; i < maxSteps; i++) {
		glm::ivec3 voxPos = (pos + front * (i * stepSize));
		Voxel vox = VoxelLookup[getVoxel(voxPos)];
		if (vox.val < 0)
		{
			return voxPos;
		}

	}


	return {};
}

void World::placeVoxel(VoxelKey vox,Camera& camera) {
	glm::vec3 front = camera.getFront();
	glm::vec3 pos = camera.getPosition();

	const float stepSize = 0.1;
	const int maxSteps = 10000;

	for (int i = 0; i < maxSteps; i++) {
		glm::vec3 voxPos = (pos + front * (i * stepSize));
		VoxelKey& lookingVox = getVoxel(glm::floor(voxPos));
		
		if (VoxelLookup[lookingVox].val < 0) {

			if (VoxelLookup[vox].val > 0) {
				lookingVox = vox;
				loadNeighbouringChunks(voxPos);
			}
			else {
				glm::vec3 newPos = voxPos - front * stepSize;
				VoxelKey& nearVox = getVoxel(glm::floor(newPos));
				nearVox = vox;

				loadNeighbouringChunks(newPos);
			}
			return;
		}

	}
}

void World::addEntity(Entity* entity){
	entityList.push_back(std::unique_ptr<Entity>(entity));
}



void World::loadNeighbouringChunks(const glm::ivec3& pos) {
	glm::ivec3 chunkPos = getChunkPos(pos);
	glm::ivec3 localPos = getLocalPos(pos);

	for (auto check : chunkNeighboursTable) {
		if (glm::all(glm::lessThanEqual(check-glm::ivec3(1),localPos))) {
			loadChunk(chunkPos - check);
		}
	}
	
	loadChunk(chunkPos);
}


unsigned int World::generateTextureFromNoise(const std::string& noiseString) {
	const int xDim = 256, yDim = 256;
	unsigned int texture;

	const FastNoise::SmartNode<> noise = FastNoise::NewFromEncodedNodeTree(noiseString.c_str());
	auto data = std::make_unique<std::array<float, xDim*yDim>>();
	noise->GenTileable2D(data->data(), xDim, yDim, 0.05f, 1337);
	glGenTextures(1, &texture);

	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, xDim, yDim, 0, GL_RED, GL_FLOAT, data->data());
	glGenerateMipmap(GL_TEXTURE_2D);

	glBindTexture(1, texture);

	return texture;
}

void World::drawQuad() {
	OpenGLCommon::drawQuad();
}



void World::drawClouds(const Camera& camera) {
	static Shader cloudShader("Cloud.fs", "Cloud.vs");
	const int CloudHeight = 100;
	const int CloudWidth = 2000;
	static unsigned int cloudTexture = generateTextureFromNoise("FwAAAIC/AACAPwAAAAAAAIA/DQADAAAAAAAAQAcAAArXIz4Aj8L1PQ==");

	glm::mat4 model(1);
	model = glm::translate(model, glm::vec3(camera.getPosition().x, CloudHeight, camera.getPosition().z));
	model = glm::scale(model, glm::vec3(CloudWidth, 1, CloudWidth));
	model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1, 0, 0));

	cloudShader.use();
	cloudShader.setMat4("model", model);
	cloudShader.setFloat("time", glfwGetTime());
	cloudShader.setFloat("cloudSpeed", 50);
	cloudShader.setVec2("windDirection", glm::normalize(glm::vec2(1, 0)));
	cloudShader.setInt("cloudTexture", 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, cloudTexture);
	

	glDisable(GL_CULL_FACE);
	drawQuad();
	glEnable(GL_CULL_FACE);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}




void World::update(btDiscreteDynamicsWorld* physicsWorld,const Camera& camera) {

	for (auto chunk = chunks.cbegin(); chunk != chunks.cend(); ) {
		chunk->second->update();
	
		float magnitude = glm::distance(glm::vec3(chunk->second->getChunkPos() * ChunkSize), camera.getPosition());
		if (magnitude > renderDistance) {
			if ((chunk->second.use_count() == 1) && chunk->second->safeToDelete()) {
				chunks.erase(chunk++);
			}
			else {
				chunk++;
			}
		}
		else {
			chunk++;
		}


	}



	std::set<btRigidBody*> loadedChunks;
	for (auto& entity : entityList) {
		float size = entity->getSize();
		glm::vec3 pos = entity->getPosition();

		static const glm::vec3 offsets[] = { {1,1,1},{-1,1,1},{1,-1,1},{-1,-1,1}, {1,1,-1},{-1,1,-1},{1,-1,-1},{-1,-1,-1} };
		bool isLoaded = true;
		for (auto offset : offsets) {
			glm::vec3 chunkPos = getChunkPos(pos + (size+3)*offset);
			
			
			if (chunks.count(chunkPos) != 0) {
				auto chunk = chunks.at(chunkPos);
				if (!chunk->isLoaded()) {
					isLoaded = false;
					break;
				}

				btRigidBody* meshBody = chunk->getPhysicsBody();
				if (meshBody != nullptr) {
					if (loadedChunks.count(meshBody) != 1) {
						physicsWorld->addRigidBody(meshBody);
						loadedChunks.insert(meshBody);
					}
				}
			}
			else {
				isLoaded = false;
				break;
			}
		}
		btRigidBody* body = entity->getRigidBody();
		if (isLoaded) {
			
			body->activate();
			physicsWorld->addRigidBody(body);
		}
		else {
			body->setActivationState(0);
		}

	}
	physicsWorld->stepSimulation(1 / 60.0f, 10);
	physicsWorld->debugDrawWorld();

	for (auto& entity : entityList) {
		physicsWorld->removeRigidBody(entity->getRigidBody());
	}

	for (auto body : loadedChunks) {
		physicsWorld->removeRigidBody(body);
	}

	

}

void World::drawEntities(Shader& shader, const Camera& camera) {
	shader.use();
	shader.setMat4("view", camera.getViewMatrix());
	globalLighting.bindShadowTexture();
	for (auto& entity : entityList) {
		entity->draw(shader, camera);
	}
}

btDiscreteDynamicsWorld* World::getPhysicsWorld() {
	return physicsWorld.physicsWorld.get();
}

void World::drawDirectionalShadows(const Camera& camera) {
	static Shader shadowShader("ChunkDirShadow.fs","ChunkDirShadow.vs");
	shadowShader.use();

	
	globalLighting.setDirectionalShadowMatrices(shadowShader, camera.getPosition());
	globalLighting.bindShadowBuffer();
		glDisable(GL_CULL_FACE);
		drawChunks(shadowShader,camera.getPosition(),renderDistance);
		glEnable(GL_CULL_FACE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	globalLighting.blurShadows();
	

}


void World::drawPointShadows(const Camera& camera) {
	static Shader shadowShader("ChunkPointShadow.fs", "ChunkPointShadow.vs", "ChunkPointShadow.gs");
	shadowShader.use();

	for (auto& entity : entityList) {
		if (entity->hasPointLight()) {
			PointLight* light = entity->getPointLight();
		

			light->setShadowMatrices(shadowShader);
			light->bindShadowBuffer();
			pointLightList.push_back(light);
			drawChunks(shadowShader, light->getPosition(), (light->getRadius() / ChunkSize) + 2);
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}
	}

	return;
}
#include <glm/gtc/integer.hpp>
#include <map>
#include <unordered_map>
#include <memory>
#include <FastNoise/FastNoise.h>
#include <future>
#include <array>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include "World.h"
#include "Chunk.h"
#include "Camera.h"

#include <optional>
#include <set>

inline glm::ivec3 World::getChunkPos(const glm::vec3& pos) {
	return glm::floor(glm::vec3(pos) / glm::vec3(ChunkSize));
}
inline glm::ivec3 World::getLocalPos(const glm::vec3& pos) {
	return glm::mod(glm::vec3(pos), glm::vec3(ChunkSize));
}



World::World() {
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

int renderDistance = 30*ChunkSize;
void World::drawChunks(Shader& shader, const Camera& camera) {
	shader.use();
	double time = glfwGetTime();

	shader.setFloat("time", time);
	shader.setInt("translucent", 0);
	shader.setVec3("cameraPos", camera.Position);
	globalLighting.setShaderUniforms(shader);
	globalLighting.setDirectionalShadowMatrices(shader, camera.Position);
	globalLighting.bindDirectionalShadowTexture();
	
	for (auto chunk = chunks.cbegin(); chunk != chunks.cend(); ) {
		float magnitude = glm::distance(glm::vec3(chunk->second->getChunkPos() * ChunkSize), camera.Position);
		if (magnitude > renderDistance) {
			//Only delete the chunk if no other thread has it in use
			if ((chunk->second.use_count() == 1) && chunk->second->safeToDelete()) {
				chunks.erase(chunk++);
			}
			else {
				chunk++;
			}
		}
		else {
			chunk->second->draw(shader);
			++chunk;
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
	glm::vec3 front = camera.Front;
	glm::vec3 pos = camera.Position;

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
	glm::vec3 front = camera.Front;
	glm::vec3 pos = camera.Position;

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



void World::loadNeighbouringChunks(glm::ivec3 pos) {
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
	struct QuadVertex {
		glm::vec3 pos;
		glm::vec2 tex;
	};
	static QuadVertex quadVertices[] = {
		// positions          // colors      // texture coords
		{{1.0f,  1.0f, 0.0f},   {1.0f, 1.0f}},   // top right
		{{1.0f, -1.0f, 0.0f},   {1.0f, 0.0f}},   // bottom right
		{{-1.0f, -1.0f, 0.0f},  {0.0f, 0.0f}},   // bottom left

		{{1.0f,  1.0f, 0.0f},   {1.0f, 1.0f}},   // top right
		{{-1.0f, -1.0f, 0.0f},  {0.0f, 0.0f}},   // bottom left
		{{-1.0f,  1.0f, 0.0f},  {0.0f, 1.0f}},    // top left 
	};

	static unsigned int VAO = 0, VBO = 0;
	if (VAO == 0) {
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);

		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(QuadVertex), quadVertices, GL_STATIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(decltype(quadVertices[0])), (void*)(offsetof(QuadVertex, pos)));

		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(decltype(quadVertices[0])), (void*)(offsetof(QuadVertex, tex)));
		glBindVertexArray(0);
	}

	glBindVertexArray(VAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
}

void World::drawClouds(Shader& shader, const Camera& camera) {

	const int CloudHeight = 100;
	const int CloudWidth = 1000;

	


	static unsigned int cloudTexture = generateTextureFromNoise("FwAAAIC/AACAPwAAAAAAAIA/DQADAAAAAAAAQAcAAArXIz4Aj8L1PQ==");

	glm::mat4 model(1);
	model = glm::translate(model, glm::vec3(camera.Position.x, CloudHeight, camera.Position.z));
	model = glm::scale(model, glm::vec3(CloudWidth, 1, CloudWidth));
	model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1, 0, 0));

	shader.use();
	shader.setMat4("model", model);
	shader.setFloat("time", glfwGetTime());
	shader.setFloat("cloudSpeed", 50);
	shader.setVec2("windDirection", glm::normalize(glm::vec2(1, 0)));
	shader.setInt("cloudTexture", 0);
	shader.setInt("shadowTexture", 1);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, cloudTexture);
	globalLighting.bindDirectionalShadowTexture();
	

	glDisable(GL_CULL_FACE);
	drawQuad();
	glEnable(GL_CULL_FACE);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}




void World::update(btDiscreteDynamicsWorld* physicsWorld) {
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
	//physicsWorld->debugDrawWorld();

	for (auto& entity : entityList) {
		physicsWorld->removeRigidBody(entity->getRigidBody());
	}

	for (auto body : loadedChunks) {
		physicsWorld->removeRigidBody(body);
	}
}

void World::drawEntities(Shader& shader, const Camera& camera) {
	globalLighting.bindDirectionalShadowTexture();
	for (auto& entity : entityList) {
		entity->draw(shader, camera);
	}
}

btDiscreteDynamicsWorld* World::getPhysicsWorld() {
	return physicsWorld.physicsWorld.get();
}

void World::drawDirectionalShadows(const Camera& camera) {
	static Shader shadowShader("ChunkShadow.fs","ChunkShadow.vs");
	shadowShader.use();
	globalLighting.setDirectionalShadowMatrices(shadowShader, camera.Position);
	globalLighting.bindDirectionalShadowBuffer();
		glDisable(GL_CULL_FACE);
		drawChunks(shadowShader,camera);
		glEnable(GL_CULL_FACE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
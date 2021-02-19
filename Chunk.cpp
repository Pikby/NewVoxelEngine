
#include <iostream>>
#include <FastNoise/FastNoise.h>


#include <cstdint>
#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/detail/compute_vector_relational.hpp>
#include <future>
#include <array>


#define GLM_FORCE_AVX
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include <vector>
#include <optional>

#include "Shader.h"
#include "Array3D.h"
//#include "Perlin.h"
#include "Chunk.h";



template <typename T> int sign(T val) {
	return (T(0) < val) - (val < T(0));
}


auto Chunk::getAllRelevantVoxels(const std::array<std::shared_ptr<Chunk>, 7> chunkNeighbours) {
	auto fullVoxels = std::make_shared<CubeArray<VoxelKey, ChunkSize + 2>>();
	if (buildTask.valid()) {
		buildTask.wait();
	}

	for (auto neigh : chunkNeighbours) {
		if (neigh->buildTask.valid()) {
			neigh->buildTask.wait();
		}
	}

	//Commit all relevant voxels for the current chunk to local memory
	for (int x = 0; x < ChunkSize + 2; x++) {
		for (int y = 0; y < ChunkSize + 2; y++) {
			for (int z = 0; z < ChunkSize + 2; z++) {
				glm::ivec3 curPos(x, y, z);

				if (x < ChunkSize && y < ChunkSize && z < ChunkSize) {
					fullVoxels->set(curPos, voxelArray.get(curPos));
					continue;
				}

				bool isOutsideChunk = false;
				for (int j = 0; j < 7; j++) {
					//if (glm::all(glm::greaterThanEqual(curPos, glm::ivec3(ChunkSize * chunkNeighboursTable[j])))) { slow on cpu profiler
					if (curPos.x >= ChunkSize * chunkNeighboursTable[j].x && curPos.y >= ChunkSize * chunkNeighboursTable[j].y && curPos.z >= ChunkSize * chunkNeighboursTable[j].z) {
						if (auto neigh = chunkNeighbours[j]) {
							VoxelKey vox = neigh->getVoxel(curPos - glm::ivec3(ChunkSize * chunkNeighboursTable[j]));
							fullVoxels->set(curPos, vox);
							isOutsideChunk = true;
							break;
						}
						else {
							//std::cout << "Fatal Error finding neighbour in memory, ABORT\n";
							throw - 1;
						}
					}
				}
				if (!isOutsideChunk) {
					fullVoxels->set(curPos, voxelArray.get(curPos));
				}


			}
		}
	}
	return fullVoxels;
}

std::pair< std::shared_ptr<ChunkMesh>, std::shared_ptr<ChunkMesh>> Chunk::getAllMeshes(std::array<std::shared_ptr<Chunk>, 7> chunkNeighbours) {

	auto relevantVoxels = getAllRelevantVoxels(chunkNeighbours);
	return std::make_pair(calculateMesh(relevantVoxels, VoxelRenderStyle::Opaque), calculateMesh(relevantVoxels, VoxelRenderStyle::Translucent));
}


std::shared_ptr<ChunkMesh> Chunk::calculateMesh(std::shared_ptr<CubeArray<VoxelKey, ChunkSize + 2>> fullVoxels, VoxelRenderStyle style) {
	

	auto chunkMesh = std::make_shared<ChunkMesh>();

	//Early check to see if meshing can be skipped
	if (fullVoxels->isAllSame()) {
		return chunkMesh;
	}

	//Moving some memory to the heap to help stop stack overflow
	auto surfaceArrayPtr = std::make_unique<CubeArray<ChunkCube, ChunkSize + 2>>();
	auto& surfaceArray = *(surfaceArrayPtr.get());
	surfaceArray.reset({ -1 });


	//Using the locally cached voxels find all the surface points
	for (int x = 0; x < ChunkSize + 1; x++) {
		for (int y = 0; y < ChunkSize + 1; y++) {
			for (int z = 0; z < ChunkSize + 1; z++) {
				glm::ivec3 curPos(x, y, z);
				ChunkCube chunkCube;

				int count = 1;
				glm::vec4 color;
				for (int i = 0; i < 8; i++) {
					glm::ivec3 offset = curPos + cubeCorners[i];

					const VoxelKey key = fullVoxels->get(offset);
					const Voxel& vox = VoxelLookup[key];
					if (style == VoxelRenderStyle::Opaque && vox.color.a != 1) {
						chunkCube.voxels[i] = VoxelTypes::Empty;
					}
					else {
						chunkCube.voxels[i] = fullVoxels->get(offset);
						if (vox.val < 0) {
							if (count == 1) {
								color = VoxelLookup[chunkCube.voxels[i]].color;
							}
							else {
								//color = (color + chunkCube.voxels[i].color) / 2;
							}
							count++;
						}
					}

				}


			
				bool sameSign = true;
				for (int i = 0; i < 7; i++) {
					const Voxel& vox1 = VoxelLookup[chunkCube.voxels[i]];
					const Voxel& vox2 = VoxelLookup[chunkCube.voxels[i + 1]];
					if (style == VoxelRenderStyle::Opaque)
					{
						if ((sign(vox1.val) != sign(vox2.val))) {
							sameSign = false;
							break;
						}
					}
					else {
						if ((sign(vox1.val) != sign(vox2.val)) && (vox1.color.a != vox2.color.a)) {
							sameSign = false;
							break;
						}
					}
				}
				if (sameSign) continue;


				glm::vec3 surfacePoint(0);
				count = 1;
				for (int i = 0; i < 12; i++) {
					Voxel vox1 = VoxelLookup[chunkCube.voxels[cubeEdges[i].first]];
					Voxel vox2 = VoxelLookup[chunkCube.voxels[cubeEdges[i].second]];
					if (sign(vox1.val) == sign(vox2.val)) continue;
					float alpha = vox1.val / static_cast<float>(vox1.val - vox2.val);

					glm::vec3 p1 = curPos + cubeCorners[cubeEdges[i].first];
					glm::vec3 p2 = curPos + cubeCorners[cubeEdges[i].second];

					glm::vec3 pf = (1 - alpha) * p1 + alpha * p2;
					surfacePoint += (pf - surfacePoint) / static_cast<float>(count);

					count++;
				}

				static const glm::ivec3 dir[4] = { {0,1,8},{3,2,9},{4,5,10},{7,6,11} };
				glm::vec3 norm(0);
				for (int i = 0; i < 4; i++) {
					norm += glm::vec3(VoxelLookup[chunkCube.voxels[cubeEdges[dir[i].x].second]].val - VoxelLookup[chunkCube.voxels[cubeEdges[dir[i].x].first]].val,
						VoxelLookup[chunkCube.voxels[cubeEdges[dir[i].y].second]].val - VoxelLookup[chunkCube.voxels[cubeEdges[dir[i].y].first]].val,
						VoxelLookup[chunkCube.voxels[cubeEdges[dir[i].z].second]].val - VoxelLookup[chunkCube.voxels[cubeEdges[dir[i].z].first]].val);
				}

				if (norm != glm::vec3(0)) {
					norm = glm::normalize(norm);
				}

				uint32_t compressedColor = static_cast<uint32_t>(Color(color).getBinaryColor());
				chunkMesh->surfacePoints.push_back({ surfacePoint,norm,compressedColor });

				chunkCube.indice = chunkMesh->surfacePoints.size() - 1;
				surfaceArray.set(curPos, chunkCube);
				
			}
		}
	}

	//Using the previously obtained surface points, find approriate edges
	for (int x = 0; x < ChunkSize + 1; x++) {
		for (int y = 0; y < ChunkSize + 1; y++) {
			for (int z = 0; z < ChunkSize + 1; z++) {
				glm::ivec3 curPos(x, y, z);
				ChunkCube chunkCube = surfaceArray.get(curPos);
				int vox[8];
				for (int i = 0; i < 8; i++) {
					vox[i] = surfaceArray.get(curPos + cubeCorners[i]).indice;
				}

				static const int edges[3] = { 3,5,6 };
				//Table of all 6 tris for the 3 possible quads
				static const glm::ivec3 tris[6] = { {0,2,1},{1,2,3},{0,1,4},{4,1,5},{0,4,2},{2,4,6} };
				//check all 3 possible quads
				for (int i = 0; i < 3; i++) {
					if (sign(VoxelLookup[chunkCube.voxels[7]].val) == sign(VoxelLookup[chunkCube.voxels[edges[i]]].val)) continue;

					//Check for the correct winding order
					bool isClockwise = (sign(VoxelLookup[chunkCube.voxels[edges[i]]].val) < 0) ? false : true;

					//Create the 2 tris of the quad
					for (int j = 0; j < 2; j++) {
						int k = i * 2 + j;
						glm::ivec3 tri = glm::ivec3(vox[tris[k].x], vox[tris[k].y], vox[tris[k].z]);
						if (tri.x == -1 || tri.y == -1 || tri.z == -1) continue;
						if (!isClockwise) {
							tri = glm::ivec3(vox[tris[k].y], vox[tris[k].x], vox[tris[k].z]);
						}
						chunkMesh->indices.push_back(tri);
					}
				}
			}
		}
	}

	return chunkMesh;
}

void Chunk::deleteBufferObjects() {
	opaqueBuffer.deleteBuffers();
	translucentBuffer.deleteBuffers();
}

BufferObject Chunk::buildBufferObject(std::vector<ChunkVertex>& surfacePoints, std::vector<glm::ivec3>& indices) {



	BufferObject bufferObject;
	if (surfacePoints.size() <= 0 || indices.size() <= 0) return bufferObject;


	glGenVertexArrays(1, &bufferObject.VAO);
	glGenBuffers(1, &bufferObject.VBO);
	glGenBuffers(1, &bufferObject.EBO);
	
	glBindVertexArray(bufferObject.VAO);
		glBindBuffer(GL_ARRAY_BUFFER, bufferObject.VBO);
		glBufferData(GL_ARRAY_BUFFER, surfacePoints.size() * sizeof(decltype(surfacePoints[0])), &(surfacePoints[0]), GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufferObject.EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(decltype(indices[0])), &(indices[0]), GL_STATIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(decltype(surfacePoints[0])), (void*)(offsetof(ChunkVertex, pos)));

		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(decltype(surfacePoints[0])), (void*)(offsetof(ChunkVertex, norm)));
	
		glEnableVertexAttribArray(2);
		glVertexAttribIPointer(2, 1, GL_INT, sizeof(decltype(surfacePoints[0])), (void*)(offsetof(ChunkVertex, color)));
	glBindVertexArray(0);

	bufferObject.verticesCount = surfacePoints.size();
	bufferObject.indicesCount = indices.size() * 3;


	return bufferObject;
	chunkFlag = ChunkFlags::LoadedToGPU;
}

void Chunk::mesh() {
	chunkFlag = ChunkFlags::QueuedToMesh;

	std::array<std::shared_ptr<Chunk>,7> chunkNeighbours;
	for (int i = 0; i < 7;i++) {
		chunkNeighbours[i] = this->chunkNeighbours[i].lock();
	}

	if (!meshTask.valid()) {
		meshTask = std::async(std::launch::async, &Chunk::getAllMeshes, this, chunkNeighbours);
	}

}


void Chunk::draw(Shader& shader){
	if (buildTask.valid()) {
		if (buildTask.wait_for(std::chrono::seconds(0)) == std::future_status::ready){
			buildTask.get();
		}
	}


	if (meshTask.valid()) {
		if (meshTask.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
			try {
				auto chunkMesh = meshTask.get();

				deleteBufferObjects();
				opaqueBuffer = buildBufferObject(chunkMesh.first->surfacePoints, chunkMesh.first->indices);
				translucentBuffer = buildBufferObject(chunkMesh.second->surfacePoints, chunkMesh.second->indices);
			}
			catch (...) {
				//Meshbuilding failed early do to surrounding chunk deconstruction
			}
		}
	}

	if (opaqueBuffer.VAO == 0) return;
	glm::mat4 model = glm::translate(glm::mat4(1), glm::vec3(chunkPos * ChunkSize));
	shader.setMat4("model", model);

	opaqueBuffer.drawBuffer();


}

void Chunk::drawTranslucent(Shader& shader) {
	if (translucentBuffer.VAO == 0) return;
	glm::mat4 model = glm::translate(glm::mat4(1), glm::vec3(chunkPos * ChunkSize));
	shader.setMat4("model", model);
	translucentBuffer.drawBuffer();
}


void Chunk::setNeighbours(const glm::ivec3& pos,std::unordered_map<glm::ivec3, std::shared_ptr<Chunk>>& chunks) {
	for (int i = 0; i < 7; i++) {
		glm::ivec3 offset = pos + chunkNeighboursTable[i];
		chunkNeighbours[i] = chunks[offset];
	}
}

bool Chunk::hasMesh() {
	return !(chunkFlag == ChunkFlags::LoadedInRAM);
}

Chunk::~Chunk(){
	deleteBufferObjects();
}


void generateEnvironmentObjects() {
	for (int x = 0; x < ChunkSize; x++) {
		for (int z = 0; z < ChunkSize; z++) {

		}
	}
}



void Chunk::generateChunk() {
	static const FastNoise::SmartNode<> perlinNoise = FastNoise::NewFromEncodedNodeTree("EQADAAAAAAAAQBAAAAAAPxkAEwCamRk/DQAEAAAAAAAgQAkAAAAAAD8AAAAAPwEEAAAAAAAAAEBAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAgD8AAAAAPwAAAAAA");
	float heightMap[ChunkSize * ChunkSize];
	perlinNoise->GenUniformGrid3D(heightMap, chunkPos.x*ChunkSize, 0, chunkPos.z*ChunkSize, ChunkSize, 1, ChunkSize, 0.005f, 1337);

	bool isEmpty = true;
	for (int x = 0; x < ChunkSize; x++) {
		for (int z = 0; z < ChunkSize; z++) {
			glm::vec3 realCoords = glm::vec3(x, 0, z) + glm::vec3(chunkPos * ChunkSize);
			
			double height = heightMap[x + z * ChunkSize];
			for (int y = 0; y < ChunkSize; y++) {
				glm::vec3 realCoords = glm::vec3(x, y,z) + glm::vec3(chunkPos * ChunkSize);
	
				int heightI = height * 100;
				VoxelKey& vox = getVoxel(glm::ivec3(x, y, z));
				if (realCoords.y < heightI) {
					vox = VoxelTypes::Full;
				}
				else if (realCoords.y < (-150 + rand() % 2)) {
					vox = VoxelTypes::Water;
				}
			}
		}
	}
	generateEnvironmentObjects();
}


Chunk::Chunk(const glm::vec3& pos) : voxelArray(VoxelTypes::Empty), chunkPos(pos) {
	buildTask = std::async(std::launch::async, &Chunk::generateChunk, this);
	chunkFlag = ChunkFlags::LoadedInRAM;
}

bool Chunk::safeToDelete() {
	return !buildTask.valid() && !meshTask.valid();
}

VoxelKey& Chunk::getVoxel(const glm::ivec3& pos) {
	return voxelArray.get(pos);
}


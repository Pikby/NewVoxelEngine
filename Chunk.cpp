#include <cstdint>
#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/detail/compute_vector_relational.hpp>
#include <future>


#include "Shader.h"
#include "Array3D.h"
#include <vector>

#include "Chunk.h";
#include <optional>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

template <typename T> int sign(T val) {
	return (T(0) < val) - (val < T(0));
}





std::shared_ptr<ChunkMesh> Chunk::calculateMesh() {
	static const glm::ivec3 offsets[8] = { {0,0,0},{1,0,0},{0,1,0},{1,1,0}, {0,0,1},{1,0,1},{0,1,1},{1,1,1} };
	static const glm::ivec3 chunkNeighboursTable[7] = { {1,1,1},{0,1,1},{1,0,1},{1,1,0},{1,0,0},{0,1,0},{0,0,1}, };
	CubeArray<Voxel, ChunkSize + 2> fullVoxels;

	for (int x = 0; x < ChunkSize + 2; x++) {
		for (int y = 0; y < ChunkSize + 2; y++) {
			for (int z = 0; z < ChunkSize + 2; z++) {
				glm::ivec3 curPos(x, y, z);

				if (x < ChunkSize && y < ChunkSize && z < ChunkSize) {
					fullVoxels.set(curPos, voxelArray.get(curPos));
					continue;
				}

				bool isOutsideChunk = false;
				for (int j = 0; j < 7; j++) {
					//if (glm::all(glm::greaterThanEqual(curPos, glm::ivec3(ChunkSize * chunkNeighboursTable[j])))) { slow on cpu profiler
					if(curPos.x >= ChunkSize * chunkNeighboursTable[j].x && curPos.y >= ChunkSize * chunkNeighboursTable[j].y && curPos.z >= ChunkSize * chunkNeighboursTable[j].z){
						if (auto neigh = chunkNeighbours[j].lock()) {
							fullVoxels.set(curPos, neigh->getVoxel(curPos - glm::ivec3(ChunkSize * chunkNeighboursTable[j])));
							isOutsideChunk = true;
							break;
						}
						else {
							std::cout << "Fatal Error finding neighbour in memory, ABORT\n";
							throw - 1;
						}
					}
				}
				if (!isOutsideChunk) {
					fullVoxels.set(curPos, voxelArray.get(curPos));
				}


			}
		}
	}

	//Moving some memory to the heap to help stop stack overflow
	auto surfaceArrayPtr = std::make_unique<CubeArray<ChunkCube, ChunkSize + 2>>();
	auto& surfaceArray = *(surfaceArrayPtr.get());
	surfaceArray.reset({ -1 });


	
	auto chunkMesh = std::make_shared<ChunkMesh>();
	for (int x = 0; x < ChunkSize + 1; x++) {
		for (int y = 0; y < ChunkSize + 1; y++) {
			for (int z = 0; z < ChunkSize + 1; z++) {
				glm::ivec3 curPos(x, y, z);
				ChunkCube chunkCube;

				

				for (int i = 0; i < 8; i++) {
					glm::ivec3 offset = curPos + offsets[i];
					chunkCube.voxels[i] = fullVoxels.get(offset);
				}

				bool sameSign = true;
				for (int i = 0; i < 7; i++) {
					if (sign(chunkCube.voxels[i].val) != sign(chunkCube.voxels[i + 1].val)) {
						sameSign = false;
						break;
					}
				}
				if (sameSign) continue;

				static const std::pair<int, int> edges[12] = { {0,1},{0,2},{1,3},{2,3},{4,5},{4,6},{5,7},{6,7},{0,4},{1,5},{2,6},{3,7} };
		
				glm::vec3 surfacePoint;
				int count = 1;
				for (int i = 0; i < 12; i++) {
					Voxel vox1 = chunkCube.voxels[edges[i].first];
					Voxel vox2 = chunkCube.voxels[edges[i].second];
					if (sign(vox1.val) == sign(vox2.val)) continue;
					float alpha = vox1.val / static_cast<float>(vox1.val - vox2.val);

					glm::vec3 p1 = curPos + offsets[edges[i].first];
					glm::vec3 p2 = curPos + offsets[edges[i].second];

					glm::vec3 pf = (1 - alpha) * p1 + alpha * p2;
					surfacePoint += (pf - surfacePoint) / static_cast<float>(count);
					count++;
				}



				static const glm::ivec3 dir[4] = { {0,1,8},{3,2,9},{4,5,10},{7,6,11} };
				glm::vec3 norm(0);
				for (int i = 0; i < 4; i++) {
					norm += glm::vec3(chunkCube.voxels[edges[dir[i].x].second].val - chunkCube.voxels[edges[dir[i].x].first].val,
						chunkCube.voxels[edges[dir[i].y].second].val - chunkCube.voxels[edges[dir[i].y].first].val,
						chunkCube.voxels[edges[dir[i].z].second].val - chunkCube.voxels[edges[dir[i].z].first].val);
				}


				if (norm != glm::vec3(0)) {

					norm = glm::normalize(norm);
				}





				chunkMesh->surfacePoints.push_back({ surfacePoint,norm });

				chunkCube.indice = chunkMesh->surfacePoints.size() - 1;
				surfaceArray.set(curPos, chunkCube);
			}
		}
	}


	for (int x = 0; x < ChunkSize + 1; x++) {
		for (int y = 0; y < ChunkSize + 1; y++) {
			for (int z = 0; z < ChunkSize + 1; z++) {
				glm::ivec3 curPos(x, y, z);
				ChunkCube chunkCube = surfaceArray.get(curPos);
				int vox[8];
				for (int i = 0; i < 8; i++) {
					vox[i] = surfaceArray.get(curPos + offsets[i]).indice;
				}

				static const int edges[3] = { 3,5,6 };
				static const glm::ivec3 tris[6] = { {0,2,1},{1,2,3},{0,1,4},{4,1,5},{0,4,2},{2,4,6} };
				for (int i = 0; i < 3; i++) {
					if (sign(chunkCube.voxels[7].val) == sign(chunkCube.voxels[edges[i]].val)) continue;


					bool isClockwise = (sign(chunkCube.voxels[edges[i]].val) < 0) ? false : true;

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

	//return meshPair(surfacePoints,indices);
	chunkFlag = ChunkFlags::Meshed;
	return chunkMesh;

}

void Chunk::buildBufferObjects(std::vector<ChunkVertex>& surfacePoints, std::vector<glm::ivec3>& indices) {



	if (surfacePoints.size() <= 0 || indices.size() <= 0) return;



	chunkFlag = ChunkFlags::Built;
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);


	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);
	

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);



	glBufferData(GL_ARRAY_BUFFER, surfacePoints.size() * sizeof(decltype(surfacePoints[0])), &(surfacePoints[0]), GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(decltype(surfacePoints[0])), (void*)(offsetof(ChunkVertex, pos)));

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(decltype(surfacePoints[0])), (void*)(offsetof(ChunkVertex, norm)));
	
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(decltype(indices[0])), &(indices[0]), GL_STATIC_DRAW);
	glBindVertexArray(0);

	verticesCount = surfacePoints.size();
	indicesCount = indices.size() * 3;
}

void Chunk::mesh() {

	//calculateMesh();
	if (chunkFlag == ChunkFlags::Empty) return;
	meshTask = std::async(std::launch::async,&Chunk::calculateMesh,this);


}


void Chunk::draw(Shader& shader){
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	if (meshTask.valid() && chunkFlag == ChunkFlags::Meshed) {
		auto chunkMesh = meshTask.get();
		buildBufferObjects(chunkMesh->surfacePoints,chunkMesh->indices);
	}
	if (chunkFlag != ChunkFlags::Built) return;
	glm::mat4 model = glm::translate(glm::mat4(1), glm::vec3(chunkPos * ChunkSize));
	shader.setMat4("model", model);
	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, indicesCount, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}


//All neighbours should exist in the chunktable before calling this function
void Chunk::setNeighbours(const glm::ivec3& pos,std::unordered_map<glm::ivec3, std::shared_ptr<Chunk>>& chunks) {
	static const glm::ivec3 chunkNeighboursTable[7] = { {1,1,1},{0,1,1},{1,0,1},{1,1,0},{1,0,0},{0,1,0},{0,0,1}, };
	for (int i = 0; i < 7; i++) {
		glm::ivec3 offset = pos + chunkNeighboursTable[i];
		chunkNeighbours[i] = chunks[offset];
	}
}

Chunk::~Chunk(){
	std::cout << "Destroying Chunk\n";
	glDeleteBuffers(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);

}

Chunk::Chunk(const glm::vec3& pos) : voxelArray(Empty), chunkPos(pos) {

	bool isEmpty = true;
	for (int x = 0; x < ChunkSize; x++) {
		for (int y = 0; y < ChunkSize; y++) {
			for (int z = 0; z < ChunkSize; z++) {

				if (chunkPos.y != 0) continue;

				Voxel& vox = getVoxel(glm::ivec3(x, y, z));


				if ((rand() % 2) == 0) {
					vox = Full;
				}
				if(y == 4) vox = Full;
				
				/*
				if ((std::pow((x-8),2) + std::pow((y-8),2) + std::pow((z-8),2)) < std::pow(4,2)) {
					voxelArray.set(x, y, z, Full);
				}
				*/
				
				
				/*
				
				if (abs(x - 8) < 4 && abs(y - 8) < 4 && abs(z - 8) < 4) {
					voxelArray.set(x, y, z, Full);
				}
				*/
				if (vox != Empty) isEmpty = false;
				
			}
		}
	}
	

	//chunkFlag = (isEmpty == true) ? ChunkFlags::Empty : ChunkFlags::Loaded;
	chunkFlag = ChunkFlags::Loaded;
}

Voxel& Chunk::getVoxel(const glm::ivec3& pos) {
	return voxelArray.get(pos);
}


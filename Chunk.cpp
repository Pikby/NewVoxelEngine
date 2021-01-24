#include <cstdint>
#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/detail/compute_vector_relational.hpp>


#include "Shader.h"
#include "Array3D.h"
#include <vector>

#include "Chunk.h";
#include <optional>

template <typename T> int sign(T val) {
	return (T(0) < val) - (val < T(0));
}


struct ChunkVertex {
	glm::vec3 pos;
	glm::vec3 norm;
};

void Chunk::mesh() {
	CubeArray<int, ChunkSize> surfaceArray(-1);
	std::vector<ChunkVertex> surfacePoints;
	for (int x = 0; x < ChunkSize-1; x++) {
		for (int y = 0; y < ChunkSize-1; y++) {
			for (int z = 0; z < ChunkSize-1; z++) {

				glm::ivec3 curPos(x, y, z);
				//surfaceArray.set(curPos, - 1);
				
				Voxel vox[8];
				static const glm::ivec3 offsets[8] = { {0,0,0},{1,0,0},{0,1,0},{1,1,0}, {0,0,1},{1,0,1},{0,1,1},{1,1,1} };
				for (int i = 0; i < 8; i++) {
					vox[i] = voxelArray.get(curPos + offsets[i]);
				}

				bool sameSign = true;
				for (int i = 0; i < 7; i++) {
					if (sign(vox[i].val) != sign(vox[i + 1].val)) {
						sameSign = false;
						break;
					}
				}
				if (sameSign) continue;

				static const std::pair<int, int> edges[12] = {{0,1},{0,2},{1,3},{2,3},{4,5},{4,6},{5,7},{6,7},{0,4},{1,5},{2,6},{3,7}};
				std::vector<glm::vec3> points;
				for (int i = 0; i < 12; i++) {
					Voxel vox1 = vox[edges[i].first];
					Voxel vox2 = vox[edges[i].second];
					if (sign(vox1.val) == sign(vox2.val)) continue;
					float alpha = vox1.val / static_cast<float>(vox1.val - vox2.val);

					glm::vec3 p1 = curPos + offsets[edges[i].first];
					glm::vec3 p2 = curPos + offsets[edges[i].second];

					glm::vec3 pf = (1 - alpha) *p1 + alpha * p2;
					points.push_back(pf);
				}


				/*
				const int xDir[4] = { 0,3,4,7 };
				const int yDir[4] = { 1,2,5,6 };
				const int zDir[4] = { 8,9,10,11 };
				*/
				static const glm::ivec3 dir[4] = { {0,1,8},{3,2,9},{4,5,10},{7,6,11} };
				glm::vec3 norm(0);
				for (int i = 0; i < 4; i++) {
					norm += glm::vec3(vox[edges[dir[i].x].second].val - vox[edges[dir[i].x].first].val, 
									  vox[edges[dir[i].y].second].val - vox[edges[dir[i].y].first].val, 
									  vox[edges[dir[i].z].second].val - vox[edges[dir[i].z].first].val);
				}

				glm::dvec3 totalPoints(0);
				for (auto point : points) {
					totalPoints += point;
				}
					
				glm::vec3 surfacePoint = totalPoints / static_cast<double>(points.size());

				norm = glm::normalize(norm);
				surfacePoints.push_back({ surfacePoint,norm });
				surfaceArray.set(curPos,surfacePoints.size()-1);
			}
		}
	}


	std::vector<glm::ivec3> indices;
	for (int x = 0; x < ChunkSize-1; x++) {
		for (int y = 0; y < ChunkSize-1; y++) {
			for (int z = 0; z < ChunkSize-1; z++) {
				glm::ivec3 curPos(x, y, z);

				static const glm::ivec3 offsets[8] = {{0,0,0},{1,0,0},{0,1,0},{1,1,0},{0,0,1},{1,0,1},{0,1,1},{1,1,1}};
				int vox[8];
				for (int i = 0; i < 8; i++) {
					vox[i] = surfaceArray.get(curPos + offsets[i]);
				}

				static const glm::ivec3 tris[6] = {{0,2,1},{2,3,1},{0,1,4},{1,5,4},{0,4,2},{4,6,2}};
				for (int i = 0; i < 6; i++) {			
					glm::ivec3 tri = glm::ivec3(vox[tris[i].x], vox[tris[i].y], vox[tris[i].z]);
					if (tri.x == -1 || tri.y == -1 || tri.z == -1) continue;

					
					glm::vec3 norm = surfacePoints[vox[tris[i].x]].norm + surfacePoints[vox[tris[i].y]].norm + surfacePoints[vox[tris[i].z]].norm;
					glm::vec3 expectedNorm = glm::cross(surfacePoints[vox[tris[i].x]].pos - surfacePoints[vox[tris[i].y]].pos, surfacePoints[vox[tris[i].x]].pos - surfacePoints[vox[tris[i].z]].pos);
					bool isClockwise = (glm::dot(norm,expectedNorm) > 0) ? true : false;
					
					if (!isClockwise) {
						tri = glm::ivec3(vox[tris[i].y], vox[tris[i].x], vox[tris[i].z]);
					}
					indices.push_back(tri);
				}
			}
		}
	}



	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	glBufferData(GL_ARRAY_BUFFER, surfacePoints.size() * sizeof(decltype(surfacePoints[0])), &surfacePoints[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(decltype(surfacePoints[0])), (void*)(offsetof(ChunkVertex, pos)));

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(decltype(surfacePoints[0])), (void*)(offsetof(ChunkVertex, norm)));

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(decltype(indices[0])), &indices[0], GL_STATIC_DRAW);
	glBindVertexArray(0);

	verticesCount = surfacePoints.size();
	indicesCount = indices.size()*3;
}


void Chunk::draw(Shader& shader){
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, indicesCount, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

Chunk::Chunk() : voxelArray(Empty) {
	for (int x = 0; x < ChunkSize; x++) {
		for (int y = 0; y < ChunkSize; y++) {
			for (int z = 0; z < ChunkSize; z++) {
				if ((std::pow((x-8),2) + std::pow((y-8),2) + std::pow((z-8),2)) < std::pow(4,2)) {
					voxelArray.set(x, y, z, Full);
				}			
			}
		}
	}
	voxelArray.set(8, 8, 8, Full);
}




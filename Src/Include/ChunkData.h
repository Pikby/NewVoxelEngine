#pragma once
#include <glm/glm.hpp>
#include "../../Include/Voxel.h"
#include "Chunk.h"
class ChunkData {

	std::unique_ptr<CubeArray<VoxelKey, ChunkSize>> data = nullptr;
	VoxelKey compressedData;
	const VoxelKey& getVoxel(const glm::vec3& pos) const {
	
	
	};
	void compress() {

		VoxelKey same = (data->operator[](0));

		for (int i = 1; i < data->size(); i++) {
			if (same != data->operator[](i)) {
				return;
			}
		}

		data = nullptr;
		compressedData = same;

	}

};


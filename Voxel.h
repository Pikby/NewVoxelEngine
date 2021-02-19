#pragma once
#include <cstdint>
#include "Color.h"
typedef uint8_t VoxelKey;





struct Voxel {
	glm::vec4 color;
	int8_t val;

	bool operator==(Voxel& rhs) const {
		return color == rhs.color && val == rhs.val;
	}

	bool operator!=(Voxel& rhs) const {
		return !(this->operator==(rhs));
	}
};

enum VoxelTypes : uint8_t {Empty=0,Full=1,Temp=2,Water=3};
static const Voxel VoxelLookup[] = {
	{{1,0,0,1},100}, //Empty
	{ {0.7,0.7,0.7,1},-100 }, //Full
	{ {0,0,1,1},-10000}, //Temp
	{ {0.32,0.62,1,0.7},-100} //Water 
};
#pragma once


template <class T, int ArrSize>
class CubeArray{
private:
	const int arrSize = ArrSize;
	T cubeArray[ArrSize * ArrSize * ArrSize];

	inline int getArrayLocation(int x, int y, int z) const {
		return x + y * arrSize + z * arrSize*arrSize;
	}

	void copy(const CubeArray& rhs) {
		for (int x = 0; x < arrSize; x++) {
			for (int y = 0; y < arrSize; y++) {
				for (int z = 0; z < arrSize; z++) {
					set(x, y, z, rhs.get(x, y, z));
				}
			}
		}
	}

	void fill(const T& value) {
		for (int x = 0; x < arrSize; x++) {
			for (int y = 0; y < arrSize; y++) {
				for (int z = 0; z < arrSize; z++) {
					set(x, y, z, value);
				}
			}
		}
	}

public:
	CubeArray(){
		reset();
	}

	CubeArray(const T& value) {
		fill(value);
	}

	CubeArray(const CubeArray& rhs) {
		copy(rhs);
	}

	CubeArray& operator=(const CubeArray& rhs) {
		copy(rhs);

		return this;
	}

	void set(const glm::ivec3& pos,const T& val) {
		set(pos.x, pos.y, pos.z, val);
	}

	void set(int x, int y, int z,const T& val) {
		cubeArray[getArrayLocation(x,y,z)] = val;
	}

	T& get(int x, int y, int z) {
		return cubeArray[getArrayLocation(x, y, z)];
	}

	const T& get(int x, int y, int z) const {
		return cubeArray[getArrayLocation(x, y, z)];
	}

	bool isAllSame() const {
		T val = get(0, 0, 0);
		for (int x = 0; x < arrSize; x++) {
			for (int y = 0; y < arrSize; y++) {
				for (int z = 0; z < arrSize; z++) {
					if (get(x, y, z) != val) {
						return false;
					}
				}
			}
		}
		return true;
	}

	T& get(const glm::ivec3& pos) {
		return get(pos.x, pos.y, pos.z);
	}
	const T& get(const glm::ivec3& pos) const {
		return get(pos.x, pos.y, pos.z);
	}

	void reset() {
		memset(cubeArray, 0, sizeof(T) * ArrSize * ArrSize * ArrSize);
	}

	void reset(const T& value) {
		fill(value);
	}

	T* data() {
		return cubeArray;
	}

	T* const data() const {
		return cubeArray;
	}
};
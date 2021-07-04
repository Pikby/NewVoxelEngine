#pragma once
#include <glm/glm.hpp>
#include <iostream>

namespace glm {
	template<class T>
	class GLMIterator {
	private:
		const T max;
		T current;
	public:
		GLMIterator(const T& Min, const T& Max) : current(Min), max(Max) {
		}
	
		GLMIterator<T> begin() {
			return GLMIterator<T>(current, max);
		}

		GLMIterator<T> end() {
			return GLMIterator<T>(max - T(1), max - T(1));
		}

		const GLMIterator<T> begin() const {
			return GLMIterator<T>(current, max);
		}

		const GLMIterator<T> end() const {
			return GLMIteartor<T>(max - T(1), max - T(1));
		}

		GLMIterator<T>& operator++() { 
			for (int index = 0; index < current.length(); index++) {
				current[index]++;
				if (current[index] >= max[index]) {
					current[index] = 0;
				}
				else {
					break;
				}
			}
			return *this;
		}

		T& operator*() {
			return current;
		}

		friend bool operator!=(const GLMIterator<T>& lhs, const GLMIterator<T>& rhs) {
			return !(lhs == rhs);
		}

		friend bool operator==(const GLMIterator<T>& lhs, const GLMIterator<T>& rhs) {
			for (int index = 0; index < lhs.current.length(); index++) {
				if (lhs.current[index] != rhs.current[index]) {
					return false;
				}
			}
			return true;
		}
	};

	template<class T>
	static GLMIterator<T> range(const T& min,const T& max) {
		return GLMIterator(min, max);
	}

	static void test() {
		auto test = range(glm::ivec3(-16, -16, -16), glm::ivec3(16, 16, 16));
		for (auto& pos : test) {
			std::cout << pos.x << ":" << pos.y << ":" << pos.z << "\n";
		}
	}
}
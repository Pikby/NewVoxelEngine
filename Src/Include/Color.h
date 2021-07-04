#pragma once
#include <iostream>
#include <cstdint>
#include <glm/glm.hpp>

static const int byteCount = 4;
static const int offset = byteCount * (8 / 4);
static const int maxVal = int(pow(2, offset)) - 1;
class Color {
private:
	uint32_t color2byte;
	uint32_t floatColorToByte(const glm::vec4& color) const {
		return (int(color.r * maxVal) & maxVal) | ((int(color.g * maxVal) & maxVal) << offset) | ((int(color.b * maxVal) & maxVal) << offset*2) | ((int(color.a * maxVal) & maxVal) << offset*3);
	}
	glm::vec4 byteColorToFloat (const uint32_t color) const {
		glm::vec4 ret;
		ret.r = (color & maxVal)/float(maxVal);
		ret.g = ((color >> offset) & maxVal) / float(maxVal);
		ret.b = ((color >> offset*2) & maxVal) / float(maxVal);
		ret.a = ((color >> offset*3) & maxVal) / float(maxVal);
		return ret;
	}

public:
	Color() : color2byte(0){}
	Color(uint32_t color) : color2byte(color){}
	Color(const glm::vec3& color) {
		color2byte = floatColorToByte(glm::vec4(color, 1));
	}

	Color(const glm::vec4& color) {
		color2byte = floatColorToByte(color);
	}
	uint32_t getBinaryColor() const{
		return color2byte;
	}

	glm::vec4 getFloatColor() const{
		return byteColorToFloat(color2byte);
	}

	bool operator==(Color& rhs) const {
		return rhs.color2byte == this->color2byte;
	}

	bool operator!=(Color& rhs) const {
		return this->operator==(rhs);
	}

	friend std::ostream& operator<<(std::ostream& out, const Color& obj){
		out << std::hex << obj.color2byte << std::dec;
	}
};
#pragma once
#include <iostream>
#include <cstdint>
#include <glm/glm.hpp>
class Color {
private:
	uint16_t color2byte;
	uint16_t floatColorToByte(const glm::vec4& color) {
		return int(color.r * 15) & 0xF  | ((int(color.g *15) & 0xF) << 4) | (((int(color.b) * 15) & 0xF) << 8) | (((int(color.a) * 15) & 0xF) << 12);
	}
	glm::vec4 byteColorToFloat(const uint16_t color) {
		glm::vec4 ret;
		ret.r = (color & 0xF)/float(0xF);
		ret.g = ((color >> 4) & 0xF) / float(0xF);
		ret.b = ((color >> 8) & 0xF) / float(0xF);
		ret.a = ((color >> 12) & 0xF) / float(0xF);
		return ret;
	}

public:
	Color() : color2byte(0){}
	Color(uint16_t color) : color2byte(color){}
	Color(const glm::vec3& color) {
		color2byte = floatColorToByte(glm::vec4(color, 1));
	}

	Color(const glm::vec4& color) {
		color2byte = floatColorToByte(color);
	}
	uint16_t getBinaryColor(){
		return color2byte;
	}

	glm::vec4 getFloatColor() {
		return byteColorToFloat(color2byte);
	}

	bool operator==(Color& rhs) {
		return rhs.color2byte == this->color2byte;
	}

	bool operator!=(Color& rhs) {
		return this->operator==(rhs);
	}

	friend std::ostream& operator<<(std::ostream& out, const Color& obj){
		out << std::hex << obj.color2byte << std::dec;
	}
};
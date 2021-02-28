#pragma once
#include <string>

class Texture {

private:
	unsigned int id;
	std::string type;
	std::string path;
public:
	Texture(const std::string& name, const std::string& directory, const std::string& typeName);



	unsigned int getId() {
		return id;
	}

	std::string getType() {
		return type;
	}

	std::string getPath() {
		return path;
	}


};
#pragma once

#include <GL/glew.h>
#include <string>

class TextureController
{
public:
	static GLuint loadFileToTexture(const std::string &filename);
};

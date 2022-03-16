#pragma once

#include <unordered_map>
#include <GL/glew.h>

class TextureLoader;
class OpenGLFont;

class TextureManager
{
public:
	static TextureManager* Inst();
	virtual ~TextureManager();

	struct TextureInfo {
		const char* filename;
		unsigned int texID = 0;
		GLenum imageFormat = GL_BGR;
		GLint internalFormat = GL_RGB;
		OpenGLFont* font = nullptr;
	};

	unsigned int loadTexture(const TextureInfo& tInfo);

protected:
	TextureManager();

	// Set of all currently loaded textures
	std::unordered_map<unsigned int, const TextureInfo> texIdLookup;

	TextureLoader* m_loader;

	// Singleton instance
	static TextureManager* m_inst;
};

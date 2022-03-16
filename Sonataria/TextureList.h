#pragma once

#include <string>
#include <unordered_map>
#include "TextureManager.h"
#include "OpenGLFont.h"

class TextureList
{
public:
	static TextureList* Inst();
	virtual ~TextureList();

	// Call this with just a filename to retrieve textures that are pre-defined
	// Call with all info to add a new texture that was not pre-defined
	// NOTE: See top of TextureList.cpp for the pre-defined textures
	unsigned int GetTextureID(const std::string& filename, GLenum fileFormat = GL_BGR, GLint internalFormat = GL_RGB);
	TextureManager::TextureInfo GetTextureInfo(const std::string& filename, GLenum fileFormat = GL_BGR, GLint internalFormat = GL_RGB);

protected:
	TextureList();

	// List of all texture info
	static std::unordered_map<std::size_t, TextureManager::TextureInfo> textureList;
	static std::size_t nextID;

	// List of all loaded font objects
	static std::vector<OpenGLFont*> fontList;

	// Internal Helper functions
	static TextureManager::TextureInfo AddTextureInfo(const std::string& filename, GLenum fileFormat, GLint internalFormat);
	static TextureManager::TextureInfo AddTextureFont(const std::string& filename, int sizePixels = 128, unsigned long charCount = 4096);
	static TextureManager::TextureInfo FindTextureInfo(const std::string& filename);

	// Singleton instance
	static TextureList* m_inst;
};

//***********************************************
// Singleton Texture Manager class
// Written by Ben English
// benjamin.english@oit.edu
//
// For use with OpenGL and the FreeImage library
//***********************************************

#pragma once

#include <windows.h>
#include <GL/glew.h>
#include <map>

class TextureLoader
{
public:
	static TextureLoader* Inst();
	virtual ~TextureLoader();

	//load a texture an make it the current texture
	//if texID is already in use, it will be unloaded and replaced with this texture
	bool LoadTexture(const char* filename,	//where to load the file from
		const unsigned int texID,			//arbitrary id you will reference the texture by
											//does not have to be generated with glGenTextures
		GLenum image_format = GL_RGB,		//format the image is in
		GLint internal_format = GL_RGB,		//format to store the image in
		GLint level = 0,					//mipmapping level
		GLint border = 0);					//border size

	//free the memory for a texture
	bool UnloadTexture(const unsigned int texID);

	//set the current texture
	bool BindTexture(const unsigned int texID);

	//free all texture memory
	void UnloadAllTextures();

protected:
	TextureLoader();

	// Singleton model (prevent default copy constructor and assignment operator)
	TextureLoader(const TextureLoader& tm) {}
	TextureLoader& operator=(const TextureLoader& tm) {}

	// Singleton instance
	static TextureLoader* m_inst;

	// Global storage of managed textures and their identifiers
	std::map<unsigned int, GLuint> m_texID;
};

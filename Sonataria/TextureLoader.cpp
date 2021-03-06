//***********************************************
// Singleton Texture Manager class
// Written by Ben English
// benjamin.english@oit.edu
//
// For use with OpenGL and the FreeImage library
//***********************************************

#include <FreeImage.h>

#include "TextureLoader.h"
#include "Logger.h"

// Initialize static instance to null
TextureLoader* TextureLoader::m_inst(NULL);

// Retrieve the singleton instance of this class (creates it if needed)
TextureLoader* TextureLoader::Inst()
{
	if (!m_inst) {
		m_inst = new TextureLoader();
	}

	return m_inst;
}

TextureLoader::TextureLoader()
{
	// call this ONLY when linking with FreeImage as a static library
	#ifdef FREEIMAGE_LIB
		FreeImage_Initialise();
	#endif
}
	
TextureLoader::~TextureLoader()
{
	// call this ONLY when linking with FreeImage as a static library
	#ifdef FREEIMAGE_LIB
		FreeImage_DeInitialise();
	#endif

	UnloadAllTextures();
	m_inst = NULL;
}

bool TextureLoader::LoadTexture(const char* filename, const unsigned int texID, GLenum image_format, GLint internal_format, GLint level, GLint border)
{
	// image format
	FREE_IMAGE_FORMAT fif = FIF_UNKNOWN;

	// pointer to the image, once loaded
	FIBITMAP *dib(0);

	// pointer to the image data
	BYTE* bits(0);

	// image width and height
	unsigned int width(0), height(0);

	// OpenGL's image ID to map to
	GLuint gl_texID;
	
	// check the file signature and deduce its format
	fif = FreeImage_GetFileType(filename, 0);

	// if still unknown, try to guess the file format from the file extension
	if (fif == FIF_UNKNOWN) {
		fif = FreeImage_GetFIFFromFilename(filename);
	}

	//if still unkown, return failure
	if (fif == FIF_UNKNOWN) {
		return false;
	}

	// check that the plugin has reading capabilities and load the file
	if (FreeImage_FIFSupportsReading(fif)) {
		dib = FreeImage_Load(fif, filename);
	}

	// if the image failed to load, return failure
	if (!dib) {
		return false;
	}

	// retrieve the image data
	bits = FreeImage_GetBits(dib);

	//get the image width and height
	width = FreeImage_GetWidth(dib);
	height = FreeImage_GetHeight(dib);

	// if this somehow one of these failed (they shouldn't), return failure
	if ((bits == 0) || (width == 0) || (height == 0)) {
		return false;
	}
	
	// if this texture ID is in use, unload the current texture
	if (m_texID.find(texID) != m_texID.end()) {
		glDeleteTextures(1, &(m_texID[texID]));
	}

	// generate an OpenGL texture ID for this texture
	glGenTextures(1, &gl_texID);

	// store the texture ID mapping
	m_texID[texID] = gl_texID;

	// bind to the new texture ID
	glBindTexture(GL_TEXTURE_2D, gl_texID);

	// ensure word alignment is enabled
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

	// store the texture data for OpenGL use
	glTexImage2D(GL_TEXTURE_2D, level, internal_format, width, height,
		border, image_format, GL_UNSIGNED_BYTE, bits);

	// Free FreeImage's copy of the data
	FreeImage_Unload(dib);

	// Configure texture settings
	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// return success
	return true;
}

bool TextureLoader::UnloadTexture(const unsigned int texID)
{
	// if this texture ID mapped, unload it's texture, and remove it from the map
	if(m_texID.find(texID) != m_texID.end())
	{
		glDeleteTextures(1, &(m_texID[texID]));
		m_texID.erase(texID);
		return true;
	}

	// otherwise, unload failed
	return false;
}

bool TextureLoader::BindTexture(const unsigned int texID)
{
	// if this texture ID mapped, bind it's texture as current
	if (m_texID.find(texID) != m_texID.end()) {
		glBindTexture(GL_TEXTURE_2D, m_texID[texID]);
		return true;
	}

	//otherwise, binding failed
	glBindTexture(GL_TEXTURE_2D, 0);
	return false;
}

void TextureLoader::UnloadAllTextures()
{
	// start at the beginning of the texture map
	auto i = m_texID.begin();

	// Unload the textures until the end of the texture map is found
	while (i != m_texID.end()) {
		UnloadTexture(i->first);
		++i;
	}

	// clear the texture map
	m_texID.clear();
}

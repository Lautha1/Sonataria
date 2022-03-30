#include "TextureList.h"

using namespace std;

// Tool to hash strings for fast comparison (included with unordered_map)
hash<string> hasher;

// Initialize static instance to null
TextureList* TextureList::m_inst(NULL);

// Retrieve the singleton instance of this class (creates it if needed)
TextureList* TextureList::Inst()
{
	if (!m_inst) {
		m_inst = new TextureList();
	}

	return m_inst;
}

// Pre-defined, known textures (we insert these into the hash-map right away)
TextureManager::TextureInfo tempTexList[] = {
	{ "Textures/temp_TitleScreen.png", 1, GL_BGR, GL_RGB, nullptr },
	{ "Textures/temp_Background.png", 2, GL_BGR, GL_RGB, nullptr },
	{ "Textures/temp_Track.png", 3, GL_BGR, GL_RGB, nullptr },
	{ "Textures/temp_Note.png", 4, GL_BGR, GL_RGB, nullptr },
	{ "Textures/temp_hold_Note.png", 5, GL_BGR, GL_RGB, nullptr },
	{ "Textures/temp_wheel_Note.png", 6, GL_BGR, GL_RGB, nullptr },

	{ "Textures/temp_SlamLeft.png", 7, GL_BGRA, GL_RGBA, nullptr },
	{ "Textures/temp_SlamRight.png", 8, GL_BGRA, GL_RGBA, nullptr },

	{ "Textures/Judgement/temp_Miss.png", 9, GL_BGRA, GL_RGBA, nullptr },
	{ "Textures/Judgement/temp_Near.png", 10, GL_BGRA, GL_RGBA, nullptr },
	{ "Textures/Judgement/temp_Perfect.png", 11, GL_BGRA, GL_RGBA, nullptr },

	{ "Textures/MissingJacketArt.png", 12, GL_BGRA, GL_RGBA, nullptr },
	{ "Textures/MissingRoundArt.png", 13, GL_BGRA, GL_RGBA, nullptr },

	{ "Fonts/HonyaJi-Re.ttf", 14, 0, 0, nullptr },

	{ "Textures/temp_LoginScreen.png", 15, GL_BGR, GL_RGB, nullptr }
};

// Declare static members of TextureList
unordered_map<std::size_t, TextureManager::TextureInfo> TextureList::textureList;
vector<OpenGLFont*> TextureList::fontList;
std::size_t TextureList::nextID = 0;

// Protected constructor, only called once internally for singleton pattern
TextureList::TextureList()
{
	// Compute length of temporary array from above
	size_t count = ((sizeof tempTexList) / (sizeof TextureManager::TextureInfo));

	// Initialize with the reserved names from above
	for (std::size_t i = 0; i < count; i++)
	{
		// For fonts, build the font
		TextureManager::TextureInfo curInfo = tempTexList[i];
		if (strstr(curInfo.filename, ".ttf") != nullptr)
		{
			OpenGLFont* newFont = new OpenGLFont(curInfo.filename, 128, 41000UL);
			fontList.push_back(newFont);
			curInfo.font = newFont;
		}

		// Store reference to texture info
		textureList[hasher(curInfo.filename)] = curInfo;
	}

	// Initialize the nextID (assumes sequential IDs were used starting at 1)
	nextID = count + 1;
}

TextureList::~TextureList() {}

TextureManager::TextureInfo TextureList::FindTextureInfo(const string& filename)
{
	// Try to lookup by filename
	std::size_t hash = hasher(filename.c_str());
	if (textureList.count(hash) > 0) {
		return textureList[hash];
	}

	// Wasn't found
	return TextureManager::TextureInfo();
}

TextureManager::TextureInfo TextureList::AddTextureFont(const std::string& filename, int sizePixels, unsigned long charCount)
{
	OpenGLFont* newFont = new OpenGLFont(filename, sizePixels, charCount);
	fontList.push_back(newFont);

	// Build the textureInfo struct
	TextureManager::TextureInfo newTexInfo =
	{
		filename.c_str(),
		nextID,
		0,
		0,
		newFont
	};

	// Hash filename and insert the textureInfo
	textureList[hasher(filename.c_str())] = newTexInfo;
	nextID++;

	// Return the textureInfo
	return newTexInfo;
}

TextureManager::TextureInfo TextureList::AddTextureInfo(const std::string& filename, GLenum fileFormat, GLint internalFormat)
{
	// Build the textureInfo struct
	TextureManager::TextureInfo newTexInfo =
	{
		filename.c_str(),
		nextID,
		fileFormat,
		internalFormat,
		nullptr
	};

	// Hash filename and insert the textureInfo
	textureList[hasher(filename.c_str())] = newTexInfo;
	nextID++;

	// Return the textureInfo
	return newTexInfo;
}

unsigned int TextureList::GetTextureID(const std::string& filename, GLenum fileFormat, GLint internalFormat)
{
	TextureManager::TextureInfo texInfo = GetTextureInfo(filename, fileFormat, internalFormat);
	return texInfo.texID;
}

TextureManager::TextureInfo TextureList::GetTextureInfo(const std::string& filename, GLenum fileFormat, GLint internalFormat)
{
	// Search for the texture info in the loaded texture array
	TextureManager::TextureInfo myTexInfo = FindTextureInfo(filename);
	if (myTexInfo.texID == 0)
	{
		// Not found, so add it to the list
		if (filename.find(".ttf") != std::string::npos)
		{
			myTexInfo = AddTextureFont(filename, 128, 41000UL);
		}
		else
		{
			myTexInfo = AddTextureInfo(filename, fileFormat, internalFormat);
		}
	}

	// Ensure the texture is loaded
	unsigned int texID = TextureManager::Inst()->loadTexture(myTexInfo);

	// Return the internal text manager ID
	return myTexInfo;
}

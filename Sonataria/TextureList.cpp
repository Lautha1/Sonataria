#include <filesystem>
#include "Logger.h"
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
	// Game
	{ "Textures/Game/Notes/Track.png", 1, GL_BGRA, GL_RGBA, nullptr },
	{ "Textures/Game/Notes/standardNote.png", 2, GL_BGRA, GL_RGBA, nullptr },

	// TEMPORARY TEXTURES
	{ "Textures/temp_hold_Note.png", 3, GL_BGR, GL_RGB, nullptr },
	{ "Textures/temp_wheel_Note.png", 4, GL_BGR, GL_RGB, nullptr },
	{ "Textures/temp_SlamLeft.png", 5, GL_BGRA, GL_RGBA, nullptr },
	{ "Textures/temp_SlamRight.png", 6, GL_BGRA, GL_RGBA, nullptr },
	{ "Textures/Judgement/temp_Miss.png", 7, GL_BGRA, GL_RGBA, nullptr },
	{ "Textures/Judgement/temp_Near.png", 8, GL_BGRA, GL_RGBA, nullptr },
	{ "Textures/Judgement/temp_Perfect.png", 9, GL_BGRA, GL_RGBA, nullptr },
	{ "Textures/MissingJacketArt.png", 10, GL_BGRA, GL_RGBA, nullptr },

	// Backgrounds
	{ "Textures/Backgrounds/Set-Morning.png", 11, GL_BGRA, GL_RGBA, nullptr },
	{ "Textures/Backgrounds/Set-Afternoon.png", 12, GL_BGRA, GL_RGBA, nullptr },
	{ "Textures/Backgrounds/Set-Evening.png", 13, GL_BGRA, GL_RGBA, nullptr },
	{ "Textures/Backgrounds/Audience.png", 14, GL_BGRA, GL_RGB, nullptr},

	// General
	{ "Textures/General/ClosedCurtainLeft.png", 15, GL_BGRA, GL_RGBA, nullptr },
	{ "Textures/General/ClosedCurtainRight.png", 16, GL_BGRA, GL_RGBA, nullptr },
	{ "Textures/General/OpenCurtains.png", 17, GL_BGRA, GL_RGBA, nullptr },
	{ "Textures/General/Stage.png", 18, GL_BGRA, GL_RGBA, nullptr },
	{ "Textures/General/StartButton.png", 19, GL_BGRA, GL_RGBA, nullptr },
	{ "Textures/General/Wheel.png", 20, GL_BGRA, GL_RGBA, nullptr },
	{ "Textures/General/Back.png", 21, GL_BGRA, GL_RGBA, nullptr },
	{ "Textures/General/No.png", 22, GL_BGRA, GL_RGBA, nullptr },
	{ "Textures/General/Yes.png", 23, GL_BGRA, GL_RGBA, nullptr },
	{ "Textures/General/CurvySymbol.png", 24, GL_BGRA, GL_RGBA, nullptr },
	{ "Textures/General/Frame.png", 25, GL_BGRA, GL_RGBA, nullptr },
	{ "Textures/General/Rope.png", 26, GL_BGRA, GL_RGBA, nullptr },
	{ "Textures/General/Spotlights/Spotlight-Left.png", 27, GL_BGRA, GL_RGBA, nullptr },
	{ "Textures/General/Spotlights/Spotlight-Right.png", 28, GL_BGRA, GL_RGBA, nullptr },
	{ "Textures/General/Brackets/LeftBracket.png", 29, GL_BGRA, GL_RGBA, nullptr },
	{ "Textures/General/Brackets/RightBracket.png", 30, GL_BGRA, GL_RGBA, nullptr },

	// Login
	{ "Textures/Login/LifeLink.png", 31, GL_BGRA, GL_RGBA, nullptr },
	{ "Textures/Login/PosterA.png", 32, GL_BGRA, GL_RGBA, nullptr },
	{ "Textures/Login/PosterB.png", 33, GL_BGRA, GL_RGBA, nullptr },
	{ "Textures/Login/TapLifeLinkPass.png", 34, GL_BGRA, GL_RGBA, nullptr },
	{ "Textures/Login/OR.png", 35, GL_BGRA, GL_RGBA, nullptr },
	{ "Textures/Login/BeginAsGuest.png", 36, GL_BGRA, GL_RGBA, nullptr },

	// Create Profile
	{ "Textures/CreateProfile/Ribbon.png", 37, GL_BGR, GL_RGB, nullptr },
	{ "Textures/CreateProfile/RibbonBox.png", 38, GL_BGRA, GL_RGBA, nullptr },
	{ "Textures/CreateProfile/1.png", 39, GL_BGRA, GL_RGBA, nullptr },
	{ "Textures/CreateProfile/2.png", 40, GL_BGRA, GL_RGBA, nullptr },
	{ "Textures/CreateProfile/3.png", 41, GL_BGRA, GL_RGBA, nullptr },
	{ "Textures/CreateProfile/4.png", 42, GL_BGRA, GL_RGBA, nullptr },

	// Other Menuing Items
	{ "Textures/Difficulties/Easy.png", 43, GL_BGRA, GL_RGBA, nullptr },
	{ "Textures/Difficulties/Medium.png", 44, GL_BGRA, GL_RGBA, nullptr },
	{ "Textures/Difficulties/Hard.png", 45, GL_BGRA, GL_RGBA, nullptr },
	{ "Textures/Difficulties/Extreme.png", 46, GL_BGRA, GL_RGBA, nullptr },

	// Thanks For Playing
	{ "Textures/ThanksForPlaying/ThanksForPlaying.png", 47, GL_BGR, GL_RGB, nullptr },

	// Game
	{ "Textures/Game/Grades/S.png", 48, GL_BGRA, GL_RGBA, nullptr },
	{ "Textures/Game/Grades/A+.png", 49, GL_BGRA, GL_RGBA, nullptr },
	{ "Textures/Game/Grades/A.png", 50, GL_BGRA, GL_RGBA, nullptr },
	{ "Textures/Game/Grades/B.png", 51, GL_BGRA, GL_RGBA, nullptr },
	{ "Textures/Game/Grades/C.png", 52, GL_BGRA, GL_RGBA, nullptr },
	{ "Textures/Game/Grades/D.png", 53, GL_BGRA, GL_RGBA, nullptr },
	{ "Textures/Game/Bar.png", 54, GL_BGRA, GL_RGBA, nullptr },
	{ "Textures/Game/Scrubber.png", 55, GL_BGRA, GL_RGBA, nullptr },

	// Playbills
	{ "Textures/Playbills/Base Playbills/Playbill-Act1.png", 56, GL_BGRA, GL_RGBA, nullptr },
	{ "Textures/Playbills/Base Playbills/Playbill-Act1Fin.png", 57, GL_BGRA, GL_RGBA, nullptr },
	{ "Textures/Playbills/Base Playbills/Playbill-Act2.png", 58, GL_BGRA, GL_RGBA, nullptr },
	{ "Textures/Playbills/Base Playbills/Playbill-Act2Fin.png", 59, GL_BGRA, GL_RGBA, nullptr },
	{ "Textures/Playbills/Base Playbills/Playbill Fin.png", 60, GL_BGRA, GL_RGBA, nullptr },
	{ "Textures/Playbills/Song Preselect Boxes/AllBoxes.png", 61, GL_BGRA, GL_RGBA, nullptr },

	// Songs
	{ "Songs/BPMRT/jacket.png", 66, GL_BGRA, GL_RGBA, nullptr },
	{ "Songs/ChartreuseGreen/jacket.png", 67, GL_BGRA, GL_RGBA, nullptr },
	{ "Songs/Dogbite/jacket.png", 68, GL_BGRA, GL_RGBA, nullptr },
	{ "Songs/Glow/jacket.png", 69, GL_BGRA, GL_RGBA, nullptr },
	{ "Songs/Psyched Fevereiro/jacket.png", 70, GL_BGRA, GL_RGBA, nullptr },
	{ "Songs/The Great Parade/jacket.png", 71, GL_BGRA, GL_RGBA, nullptr },

	// Fonts
	{ "Fonts/HonyaJi-Re.ttf", 72, 0, 0, nullptr },
	{ "Fonts/Stayola-Regular.otf", 73, 0, 0, nullptr}
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
		else if (strstr(curInfo.filename, ".otf") != nullptr)
		{
			OpenGLFont* newFont = new OpenGLFont(curInfo.filename, 128, 256UL);
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

void TextureList::PreloadTextures() {
	logger.log("Preloading textures...");

	// Find all Jacket Arts First
	LoadJacketArts();

	// Preload texture ids
	for (auto& [key, value] : textureList) {
		GetTextureID(value.filename);
	}

	logger.log("Preloaded textures.");
}

void TextureList::LoadJacketArts() {
	std::string dir = "./Songs";

	for (auto& item : filesystem::recursive_directory_iterator(dir)) {

		if (!filesystem::is_regular_file(item.path())
			|| item.path().extension() != ".png")
			continue;

		string path = item.path().string().substr(2, string::npos);

		replace(path.begin(), path.end(), '\\', '/');

		AddTextureInfo(path, GL_BGR, GL_RGB);
	}
}

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

#include "TextureManager.h"

#include "TextureLoader.h"

using namespace std;

// Initialize static instance to null
TextureManager* TextureManager::m_inst(NULL);

// Retrieve the singleton instance of this class (creates it if needed)
TextureManager* TextureManager::Inst()
{
	if (!m_inst) {
		m_inst = new TextureManager();
	}

	return m_inst;
}

TextureManager::TextureManager()
{
	m_loader = TextureLoader::Inst();
}

TextureManager::~TextureManager() {}

unsigned int TextureManager::loadTexture(const TextureInfo& tInfo)
{
	// Sanity check
	if (tInfo.texID == 0) { return 0; }

	// Look if the proper texture is already loaded
	if (texIdLookup.count(tInfo.texID) > 0)
	{
		if (strcmp(tInfo.filename, texIdLookup[tInfo.texID].filename) == 0)
		{
			return tInfo.texID;
		}
		else
		{
			texIdLookup.erase(tInfo.texID);
		}
	}

	// Load texture and add to lookup
	m_loader->LoadTexture(tInfo.filename, tInfo.texID, tInfo.imageFormat, tInfo.internalFormat);
	texIdLookup.emplace(tInfo.texID, tInfo);

	// Return the texture manager ID
	return tInfo.texID;
}

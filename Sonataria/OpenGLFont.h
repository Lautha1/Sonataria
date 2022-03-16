#pragma once

#include <ft2build.h>
#include FT_FREETYPE_H

#include <string>
#include <map>

#include "Vectors.h"

struct Character {
	unsigned int TextureID; // ID handle of the glyph texture
	Vector2 Size;           // Size of glyph
	Vector2 Bearing;        // Offset from baseline to left/top of glyph
	long Advance;			// Offset to advance to next glyph
};

class OpenGLFont {
	public:
		OpenGLFont(const std::string& filename, int sizePixels = 128, unsigned long charCount = 4096);
		const struct Character& lookUpChar(const unsigned long charCode) const;

	protected:
		std::map<unsigned long, Character> Characters;

		void buildTextures(FT_Face ftFace, unsigned long characterCount);

		static FT_Face loadFTFont(const std::string& filename);
		static FT_Library ftLib;
};

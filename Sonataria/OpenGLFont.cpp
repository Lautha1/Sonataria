#include "OpenGLFont.h"

#include <GL/glew.h>

#include <iostream>
#include "Logger.h"
using namespace std;

FT_Library OpenGLFont::ftLib = nullptr;

Character nullCharacter = {
    0,
    Vector2(),
    Vector2(),
    0
};

OpenGLFont::OpenGLFont(const std::string& filename, int sizePixels, unsigned long charCount) {
    if (ftLib == nullptr) {
        if (FT_Init_FreeType(&ftLib))
        {
            logger.logError(L"ERROR::FREETYPE: Could not init FreeType Library");
        }
    }

    if (ftLib != nullptr) {
        FT_Face ftFace = loadFTFont(filename);
        FT_Set_Pixel_Sizes(ftFace, 0, sizePixels);

        if (ftFace != nullptr) {
            buildTextures(ftFace, charCount);
            FT_Done_Face(ftFace);
        }
    }

    logger.log(L"Font loaded successfully");
}

FT_Face OpenGLFont::loadFTFont(const std::string& filename) {
    FT_Face face;
    if (FT_New_Face(ftLib, filename.c_str(), 0, &face))
    {
        std::cout << "ERROR::FREETYPE: Failed to load font - " << filename << endl;
        return nullptr;
    }

    return face;
}

void OpenGLFont::buildTextures(FT_Face ftFace, unsigned long characterCount) {
    // disable byte-alignment restriction
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    for (FT_ULong c = 0; c < characterCount; c++)
    {
        // load character glyph 
        if (FT_Load_Char(ftFace, c, FT_LOAD_RENDER))
        {
            std::cout << "ERROR::FREETYTPE: Failed to load Glyph (" << c << ")" << std::endl;
            continue;
        }

        // generate texture
        unsigned int texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RED,
            ftFace->glyph->bitmap.width,
            ftFace->glyph->bitmap.rows,
            0,
            GL_RED,
            GL_UNSIGNED_BYTE,
            ftFace->glyph->bitmap.buffer
        );

        // set texture options
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // now store character for later use
        Character character = {
            texture,
            Vector2((float)ftFace->glyph->bitmap.width, (float)ftFace->glyph->bitmap.rows),
            Vector2((float)ftFace->glyph->bitmap_left, (float)ftFace->glyph->bitmap_top),
            ftFace->glyph->advance.x
        };
        Characters.insert(std::pair<unsigned long, Character>(c, character));
    }

    // Set pixel row alignment back to the default (word-alignment)
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
}

const Character& OpenGLFont::lookUpChar(const unsigned long charCode) const {
    std::map<unsigned long, Character>::const_iterator it;
    it = Characters.find(charCode);
    if (it == Characters.end()) {
        // Didn't find it
        logger.logError(L"Couldn't find character - " + to_wstring(charCode));
        return nullCharacter;
    }
    else {
        return (*it).second;
    }
}

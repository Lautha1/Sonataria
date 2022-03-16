#include "OpenGLText.h"
#include "Logger.h"

using namespace std;

OpenGLText::OpenGLText(const wstring& newName, const OpenGLFont& font) : QuadSprite(newName), font(font) {
    float halfWidthPixels = 1920;
    float halfHeightPixels = 1080;
    mOrtho.setOrthoFrustum(
        -16.f / 9.f * halfWidthPixels, 16.f / 9.f * halfWidthPixels,
        -1.f * halfHeightPixels, 1.f * halfHeightPixels,
        -1.f, 1.f
    );
}

void OpenGLText::render(PROJECTION projection) const {
	logger.logError(L"Do not use this render function for text!");
}

float OpenGLText::computeTextWidth(const std::string& text, GLfloat pack) const {
    float textWidth = 0.f;

    for (std::size_t i = 0; i < text.length(); i++)
    {
        Character ch = font.lookUpChar(text[i]);
        if (i != text.length() - 1)
        {
            textWidth += (ch.Advance >> 6) * pack;
        }
    }

    return textWidth;
}

float OpenGLText::computeTextWidth(const std::wstring& text, GLfloat pack) const {
    float textWidth = 0.f;

    for (std::size_t i = 0; i < text.length(); i++)
    {
        Character ch = font.lookUpChar(text[i]);
        if (i != text.length() - 1)
        {
            textWidth += (ch.Advance >> 6) * pack;
        }
    }

    return textWidth;
}

void OpenGLText::render(PROJECTION projection, const string& text, ALIGNMENT align,
    GLfloat r, GLfloat g, GLfloat b, GLfloat pack, GLfloat scale) {

    float xOffset = 0.f;
    float yOffset = 0.f;

    // Adjust offset based on alignment
    if (align != ALIGNMENT::LEFT) {
        float textWidth = computeTextWidth(text, pack);
        if (align == ALIGNMENT::RIGHT) {
            xOffset = -textWidth;
        }
        else {
            xOffset = -textWidth / 2.f;
        }
    }

    // Update the color tint in the shader program
    GLuint colorTintLoc = glGetUniformLocation(this->program, "colorTint");
    glUniform3f(colorTintLoc, r, g, b);

    Vector3 baseScale = mScale;
    Vector3 baseTranslate = mPosition;

    // iterate through all characters
    std::string::const_iterator c;
    for (c = text.begin(); c != text.end(); c++)
    {
        Character ch = font.lookUpChar(*c);

        float xpos = xOffset + ch.Size.x / 2.f - ch.Bearing.x;
        float ypos = yOffset - ch.Size.y / 2.f + ch.Bearing.y;

        float w = ch.Size.x;
        float h = ch.Size.y;

        glActiveTexture(GL_TEXTURE0);

        // render glyph texture over quad
        glBindTexture(GL_TEXTURE_2D, ch.TextureID);

        mScale.x = w * baseScale.x * scale;
        mScale.y = h * -baseScale.y;

        mPosition.x = baseTranslate.x + xpos; // / (1920 / 2);
        mPosition.y = baseTranslate.y + ypos; // / (1080 / 4);
        updateTransformation();

        QuadSprite::render(projection);

        // now advance cursors for next glyph (note that advance is number of 1/64 pixels)
        xOffset += (ch.Advance >> 6) * pack; // bitshift by 6 to get value in pixels (2^6 = 64)
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);

    mScale = baseScale;
    mPosition = baseTranslate;
}

void OpenGLText::render(PROJECTION projection, const wstring& text, ALIGNMENT align, GLfloat r, GLfloat g, GLfloat b, GLfloat pack, GLfloat scale) {

    float xOffset = 0.f;
    float yOffset = 0.f;

    // Adjust offset based on alignment
    if (align != ALIGNMENT::LEFT) {
        float textWidth = computeTextWidth(text, pack);
        if (align == ALIGNMENT::RIGHT) {
            xOffset = -textWidth;
        }
        else {
            xOffset = -textWidth / 2.f;
        }
    }

    // Update the color tint in the shader program
    GLuint colorTintLoc = glGetUniformLocation(this->program, "colorTint");
    glUniform3f(colorTintLoc, r, g, b);

    Vector3 baseScale = mScale;
    Vector3 baseTranslate = mPosition;

    // iterate through all characters
    std::wstring::const_iterator c;
    for (c = text.begin(); c != text.end(); c++)
    {
        Character ch = font.lookUpChar(*c);

        float xpos = xOffset + ch.Size.x / 2.f - ch.Bearing.x;
        float ypos = yOffset - ch.Size.y / 2.f + ch.Bearing.y;

        float w = ch.Size.x;
        float h = ch.Size.y;
        
        glActiveTexture(GL_TEXTURE0);

        // render glyph texture over quad
        glBindTexture(GL_TEXTURE_2D, ch.TextureID);
        
        mScale.x = w * baseScale.x * scale;
        mScale.y = h * -baseScale.y;

        mPosition.x = baseTranslate.x + xpos; // / (1920 / 2);
        mPosition.y = baseTranslate.y + ypos; // / (1080 / 4);
        updateTransformation();

        QuadSprite::render(projection);

        // now advance cursors for next glyph (note that advance is number of 1/64 pixels)
        xOffset += (ch.Advance >> 6) * pack; // bitshift by 6 to get value in pixels (2^6 = 64)
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);

    mScale = baseScale;
    mPosition = baseTranslate;
}

#include "TextShader.h"

#include "resource.h"
#include "OpenGLSprite.h"

TextShader::TextShader() : OpenGLShader(IDR_TEXT_VERTEX_SHADER, IDR_TEXT_FRAGMENT_SHADER) {
	texUniformLoc = 0;
	colorTintLoc = 0;
}

TextShader::~TextShader() {}

void TextShader::initAttributes() {
	glBindAttribLocation(this->shaderProgram, ATTRIB_POSITION_INDEX, "v_pos");
	glBindAttribLocation(this->shaderProgram, ATTRIB_COLOR_INDEX, "v_col");
	glBindAttribLocation(this->shaderProgram, ATTRIB_TEX_UV_INDEX, "v_uv");
}

void TextShader::initUniforms() {
	texUniformLoc = glGetUniformLocation(this->shaderProgram, "greyTex");
	glUniform1i(texUniformLoc, 0);

	colorTintLoc = glGetUniformLocation(this->shaderProgram, "colorTint");
	glUniform3f(colorTintLoc, 1.0f, 1.0f, 1.0f);
}

void TextShader::setTint(GLfloat r, GLfloat g, GLfloat b) {
	glUniform3f(colorTintLoc, r, g, b);
}

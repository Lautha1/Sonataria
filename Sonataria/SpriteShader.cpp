#include "SpriteShader.h"

#include "resource.h"

#include "OpenGLSprite.h"

SpriteShader::SpriteShader() : OpenGLShader(IDR_VERTEX_SHADER, IDR_FRAGMENT_SHADER) {}
SpriteShader::~SpriteShader() {}

void SpriteShader::initAttributes() {
	glBindAttribLocation(this->shaderProgram, ATTRIB_POSITION_INDEX, "v_pos");
	glBindAttribLocation(this->shaderProgram, ATTRIB_COLOR_INDEX, "v_col");
	glBindAttribLocation(this->shaderProgram, ATTRIB_TEX_UV_INDEX, "v_uv");
}

void SpriteShader::initUniforms() {
	texUniformLoc = glGetUniformLocation(this->shaderProgram, "colorTex");
	glUniform1i(texUniformLoc, 0);
}

#include "VideoShader.h"

#include "resource.h"
#include "OpenGLSprite.h"

VideoShader::VideoShader() : OpenGLShader(IDR_VIDEO_VERTEX_SHADER, IDR_VIDEO_FRAGMENT_SHADER) {
	textureLoc[0] = 0;
	textureLoc[1] = 0;
	textureLoc[2] = 0;
}

VideoShader::~VideoShader() {}

void VideoShader::initAttributes() {
	glBindAttribLocation(this->shaderProgram, ATTRIB_POSITION_INDEX, "v_pos");
	glBindAttribLocation(this->shaderProgram, ATTRIB_COLOR_INDEX, "v_col");
	glBindAttribLocation(this->shaderProgram, ATTRIB_TEX_UV_INDEX, "v_uv");
}

void VideoShader::initUniforms() {
	textureLoc[0] = glGetUniformLocation(this->shaderProgram, "f_yTexture");
	textureLoc[1] = glGetUniformLocation(this->shaderProgram, "f_uTexture");
	textureLoc[2] = glGetUniformLocation(this->shaderProgram, "f_vTexture");
	glUniform1i(textureLoc[0], 0);
	glUniform1i(textureLoc[1], 1);
	glUniform1i(textureLoc[2], 2);
}

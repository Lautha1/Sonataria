#pragma once
#include "OpenGLShader.h"

class VideoShader : public OpenGLShader
{
public:
	VideoShader();
	virtual ~VideoShader();

protected:
	GLint textureLoc[3];

	virtual void initAttributes() override;
	virtual void initUniforms() override;
};

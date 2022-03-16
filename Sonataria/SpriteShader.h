#pragma once
#include "OpenGLShader.h"
class SpriteShader : public OpenGLShader
{
public:
	SpriteShader();
	virtual ~SpriteShader();

protected:
	GLuint texUniformLoc;

	virtual void initAttributes() override;
	virtual void initUniforms() override;
};

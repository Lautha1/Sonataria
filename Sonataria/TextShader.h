#pragma once
#include "OpenGLShader.h"
class TextShader : public OpenGLShader
{
public:
	TextShader();
	virtual ~TextShader();

	void setTint(GLfloat r, GLfloat g, GLfloat b);

protected:
	GLuint texUniformLoc, colorTintLoc;

	virtual void initAttributes() override;
	virtual void initUniforms() override;
};

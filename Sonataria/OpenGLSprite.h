#pragma once

#include <GL/glew.h>
#include "Matrices.h"
#include <cstdio>
#include <string>
#include <vector>
#include <stack>

// Vertex attribute constants
#define ATTRIB_POSITION_INDEX	0
#define ATTRIB_COLOR_INDEX		1
#define ATTRIB_TEX_UV_INDEX		2

// Error checking for OpenGL (the old way, works on AMD)
#define CHECK_GL_ERROR() {\
	GLenum errCode; \
	const GLubyte* errString; \
	printf("Checking (%s: %d)\n", __FILE__, __LINE__); \
	if ((errCode = glGetError()) != GL_NO_ERROR) { \
		errString = gluErrorString(errCode); \
		printf("OpenGL Error: %s\n", errString); \
	}}

class OpenGLSprite
{
public:
	OpenGLSprite(const std::wstring& newName);
	virtual ~OpenGLSprite();

	void initSprite(GLuint program);
	void refreshSprite() const;
	void render(const Matrix4& mT, const Matrix4& mProj) const;

	void setTextureID(GLuint managerTexID);

	void setOpacity(float newOpacity);

	void setXStretch(float xScale = 1.0f, float xTranslate = 0.0f);
	void setYStretch(float yScale = 1.0f, float yTranslate = 0.0f);

	static void pushMatrix(const Matrix4& mModel);
	static void popMatrix();

protected:
	// Setup a new attribute pointer for the currently bound VAO
	static void createAttribPointer(GLuint attribIndex, int count, int stride, int offset);

	// Bind current texture for rendering
	void bindTexture() const;

	// Name of this sprite
	std::wstring name;

	// Children list
	static std::stack<Matrix4> matrixStack;

	// Stretching
	bool enableStretch;
	float xStretch[2], yStretch[2];

	// Opacity
	float opacity = 1.f;

	// Array of all vertex attributes
	float* pointData;
	mutable bool vertexDataChanged;

	// Number of vertices in pointData
	long vertCount;

	// Does pointData have color and UV attributes?
	bool hasColors, hasUVs;

	// The ID of the texture (as used by texture manager)
	GLuint managerTexID;

	// The OpenGL array and buffer objects
	GLuint vao, vbo;

	// The geometry render type passed to glDraw
	GLuint renderType;

	// The program being used to draw
	GLuint program;
};

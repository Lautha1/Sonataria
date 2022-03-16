#include "OpenGLSprite.h"
#include "TextureLoader.h"

#include "Logger.h"

using namespace std;

// Allocating the static matrix stack
std::stack<Matrix4> OpenGLSprite::matrixStack;

OpenGLSprite::OpenGLSprite(const wstring& newName) : name(newName) {
	pointData = nullptr;
	vertCount = 0;
	managerTexID = 0;
	hasColors = hasUVs = false;
	vao = vbo = 0;
	renderType = GL_TRIANGLES;
	program = 0;
	vertexDataChanged = false;

	enableStretch = false;
	xStretch[0] = 1.0f;
	xStretch[1] = 0.0f;
	yStretch[0] = 1.0f;
	yStretch[1] = 0.0f;
}

OpenGLSprite::~OpenGLSprite() {}

void OpenGLSprite::createAttribPointer(GLuint attribIndex, int count, int stride, int offset)
{
	glVertexAttribPointer(attribIndex, count, GL_FLOAT, GL_FALSE,
		stride, (void*)(offset * sizeof(GLfloat)));
	glEnableVertexAttribArray(attribIndex);
}

void OpenGLSprite::bindTexture() const
{
	// Bind texture for use
	glActiveTexture(GL_TEXTURE0);
	TextureLoader::Inst()->BindTexture(managerTexID);
}

void OpenGLSprite::setXStretch(float xScale, float xTrans) {
	enableStretch = true;
	xStretch[0] = xScale;
	xStretch[1] = xTrans;
}

void OpenGLSprite::setYStretch(float yScale, float yTrans) {
	enableStretch = true;
	yStretch[0] = yScale;
	yStretch[1] = yTrans;
}

void OpenGLSprite::initSprite(GLuint program)
{
	// Generate VAO and VBO
	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);

	// Pass data to GPU and setup attribute pointers
	refreshSprite();

	// Save a reference to the current program
	this->program = program;
}

void OpenGLSprite::refreshSprite() const {
	// Length of one cluster of vertex data (in bytes)
	int stride = (3 + (hasColors ? 3 : 0) + (hasUVs ? 2 : 0)) * sizeof(float);

	// Bind previouly generate VAO
	glBindVertexArray(vao);

	// Pass data into vertex buffer (copies into GPU memory)
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, vertCount * stride, pointData, GL_STATIC_DRAW);

	// Set pointers to different vertex attributes
	createAttribPointer(ATTRIB_POSITION_INDEX, 3, stride, 0); // Position
	if (hasColors) {
		createAttribPointer(ATTRIB_COLOR_INDEX, 3, stride, 3); // Color
	}
	if (hasUVs) {
		createAttribPointer(ATTRIB_TEX_UV_INDEX, 2, stride, (hasColors ? 6 : 3)); // UV
	}
}

void OpenGLSprite::setTextureID(GLuint managerTexID)
{
	// Initialize texture
	this->managerTexID = managerTexID;
}

void OpenGLSprite::pushMatrix(const Matrix4& mModel)
{
	Matrix4 modelTrans;
	if (!matrixStack.empty())
	{
		modelTrans = matrixStack.top();
	}
	modelTrans = modelTrans * mModel;
	matrixStack.push(modelTrans);
}

void OpenGLSprite::popMatrix()
{
	if (!matrixStack.empty())
	{
		matrixStack.pop();
	}
	else
	{
		logger.logError(L"Matrix stack underflow");
	}
}

void OpenGLSprite::render(const Matrix4& mT, const Matrix4& mProj) const
{
	if (vertexDataChanged) {
		refreshSprite();
		vertexDataChanged = false;
	}

	// Bind and configure the array/buffer objects
	glBindVertexArray(vao);
	glEnableVertexAttribArray(ATTRIB_POSITION_INDEX);
	if (hasColors) { glEnableVertexAttribArray(ATTRIB_COLOR_INDEX); }
	if (hasUVs) { glEnableVertexAttribArray(ATTRIB_TEX_UV_INDEX); }

	// Bind texture for use
	if (managerTexID > 0) { bindTexture(); }

	// Set stretch values
	GLint enableStretchLoc = glGetUniformLocation(this->program, "enableStretch");
	glProgramUniform1i(this->program, enableStretchLoc, enableStretch);
	if (enableStretch) {
		GLint xStretchLoc = glGetUniformLocation(this->program, "xStretch");
		GLint yStretchLoc = glGetUniformLocation(this->program, "yStretch");
		glProgramUniform2f(this->program, xStretchLoc, xStretch[0], xStretch[1]);
		glProgramUniform2f(this->program, yStretchLoc, yStretch[0], yStretch[1]);
	}

	// Set value for opacity
	GLint opacityLoc = glGetUniformLocation(this->program, "opacity");
	glProgramUniform1f(this->program, opacityLoc, opacity);

	// Pass in the depth clipping value
	// TODO: Possibly make this a member variable
	GLint depthClipLoc = glGetUniformLocation(this->program, "zClip");
	glProgramUniform1f(this->program, depthClipLoc, 5.5f);

	// Account for model hierarchy
	Matrix4 modelTrans = mT;
	if (!matrixStack.empty())
	{
		modelTrans = matrixStack.top() * modelTrans;
	}

	// Pass in the uniform model matrix
	GLint modelLoc = glGetUniformLocation(this->program, "model");
	glProgramUniformMatrix4fv(this->program, modelLoc, 1, GL_FALSE, modelTrans.get());

	// Pass in the projection matrix
	GLint projectionLoc = glGetUniformLocation(this->program, "projection");
	glProgramUniformMatrix4fv(this->program, projectionLoc, 1, GL_FALSE, mProj.get());

	// Draw points from the bound VAO with the bound shader program
	glDrawArrays(renderType, 0, vertCount);
}

void OpenGLSprite::setOpacity(float newOpacity) {
	opacity = newOpacity;
}

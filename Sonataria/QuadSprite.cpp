#include "QuadSprite.h"
#include "Vectors.h"
#include <cstring>

using namespace std;

QuadSprite::QuadSprite(const wstring& newName) : OpenGLSprite(newName) {
	init();

	// Build up the point data with struct literals
	vertCount = 4;
	structPoints = new QuadVertData[vertCount];

	//			  (  X,     Y,     Z )    ( R,    G,    B )    ( U,    V, )
	structPoints[0] = {{ -0.5f, -0.5f, 0.0f }, { 1.0f, 0.0f, 0.0f}, { 0.0f, 0.0f }};
	structPoints[1] = {{  0.5f, -0.5f, 0.0f }, { 0.0f, 1.0f, 0.0f}, { 1.0f, 0.0f }};
	structPoints[2] = {{  0.5f,  0.5f, 0.0f }, { 0.0f, 0.0f, 1.0f}, { 1.0f, 1.0f }};
	structPoints[3] = {{ -0.5f,  0.5f, 0.0f }, { 1.0f, 1.0f, 0.0f}, { 0.0f, 1.0f }};

	// Reinterpret the memory into just an array of floats
	pointData = reinterpret_cast<float*>(structPoints);
}

QuadSprite::QuadSprite(const wstring& newName, const QuadVertData* newPoints, int newVertCount) : OpenGLSprite(newName) {
	init();

	// Build up the point data with struct literals
	vertCount = newVertCount;
	structPoints = new QuadVertData[vertCount];
	memcpy(structPoints, newPoints, vertCount * sizeof(QuadVertData));

	// Reinterpret the memory into just an array of floats
	pointData = reinterpret_cast<float*>(structPoints);
}

void QuadSprite::init() {
	// Configure basic vertex data
	hasColors = hasUVs = true;
	renderType = GL_TRIANGLE_FAN;
}

void QuadSprite::setVertexLocation(int index, float x, float y, float z)
{
	if (index >= 0 && index < vertCount) {
		structPoints[index].pos.x = x;
		structPoints[index].pos.y = y;
		structPoints[index].pos.z = z;
		vertexDataChanged = true;
	}
}

void QuadSprite::setVertexPixelLocation(int index, float x, float y, float z)
{
	// Assume 1920 x 1080
	// CAUTION: (0, 0) is at pixel (960, 540) [middle of screen]
	if (index >= 0 && index < vertCount) {
		structPoints[index].pos.x = x * 2.0f / 1080.0f;
		structPoints[index].pos.y = y * 2.0f / 1080.0f;
		structPoints[index].pos.z = z * 2.0f / 1080.0f;
		vertexDataChanged = true;
	}
}

void QuadSprite::scaleUVs(float uScale, float vScale)
{
	QuadVertData* points = reinterpret_cast<QuadVertData*>(pointData);
	for (int i = 0; i < vertCount; i++)
	{
		points[i].tex *= Vector2(uScale, vScale);
	}
}

void QuadSprite::offsetUVs(float uOffset, float vOffset)
{
	QuadVertData* points = reinterpret_cast<QuadVertData*>(pointData);
	for (int i = 0; i < vertCount; i++)
	{
		points[i].tex += Vector2(uOffset, vOffset);
	}
}

void QuadSprite::render(PROJECTION projType) const
{
	if (projType == ORTHOGRAPHIC) {
		OpenGLSprite::render(mT, mOrtho);
	}
	else {
		OpenGLSprite::render(mT, mPersp);
	}
}

QuadSprite::~QuadSprite() {
	delete[] pointData;
}

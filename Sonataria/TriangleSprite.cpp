#include "TriangleSprite.h"
#include "Vectors.h"
#include <cstring>

using namespace std;

// Full vertex data (for easier initialization)
struct TriVertData {
	Vector3 pos, color;
	Vector2 tex;
};

TriangleSprite::TriangleSprite(const wstring& newName) : OpenGLSprite(newName) {
	// Configure basic vertex data
	hasColors = hasUVs = true;
	vertCount = 3;
	renderType = GL_TRIANGLES;

	// Build up the point data with struct literals
	TriVertData* points = new TriVertData[vertCount];

	//			  (  X,     Y,     Z )    ( R,    G,    B )    ( U,    V, )
	points[0] = {{  0.0f,  0.5f, 0.0f }, { 0.0f, 1.0f, 0.0f}, { 0.5f, 1.0f }};
	points[1] = {{  0.5f, -0.5f, 0.0f }, { 0.0f, 0.0f, 1.0f}, { 1.0f, 0.0f }};
	points[2] = {{ -0.5f, -0.5f, 0.0f }, { 1.0f, 0.0f, 0.0f}, { 0.0f, 0.0f }};

	// Reinterpret the memory into just an array of floats
	pointData = reinterpret_cast<float*>(points);
}

TriangleSprite::~TriangleSprite() {
	delete[] pointData;
}

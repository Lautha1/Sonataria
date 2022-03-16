#include "SlicedSprite.h"

#include "QuadSprite.h"

// TODO: Consider x stretch as well
// TODO: Code stretch values in shader so can be dynamic

SlicedSprite::SlicedSprite(const std::wstring& newName, float xStretch, float yStretch)
{
	// Allocate the slices
	slices[0] = new QuadSprite(newName + L"-top-left");
	slices[1] = new QuadSprite(newName + L"-top-mid");
	slices[2] = new QuadSprite(newName + L"-top-right");
	slices[3] = new QuadSprite(newName + L"-mid-left");
	slices[4] = new QuadSprite(newName + L"-mid-mid");
	slices[5] = new QuadSprite(newName + L"-mid-right");
	slices[6] = new QuadSprite(newName + L"-bot-left");
	slices[7] = new QuadSprite(newName + L"-bot-mid");
	slices[8] = new QuadSprite(newName + L"-bot-right");

	// Adjust the UVs
	for (int row = 0; row < 3; row++) {
		for (int col = 0; col < 3; col++) {
			slices[row * 3 + col]->scaleUVs(1 / 3.f, 1 / 3.f);
			slices[row * 3 + col]->offsetUVs(col / 3.f, row / 3.f);
		}
	}

	// Apply initial stretch values
	setStretch(xStretch, yStretch);
}

SlicedSprite::~SlicedSprite()
{
	for (int i = 0; i < 9; i++) {
		delete slices[i];
	}
}

void SlicedSprite::setStretch(float xStretch, float yStretch)
{
	// Adjust transforms to x stretch
	float midXScale = 1 / 3.f * xStretch;
	float halfMidX = 1 / 6.f + midXScale / 2.f;
	slices[0]->setXStretch(1 / 3.f, -(halfMidX));
	slices[3]->setXStretch(1 / 3.f, -(halfMidX));
	slices[6]->setXStretch(1 / 3.f, -(halfMidX));

	slices[1]->setXStretch(midXScale, 0.0f);
	slices[4]->setXStretch(midXScale, 0.0f);
	slices[7]->setXStretch(midXScale, 0.0f);

	slices[2]->setXStretch(1 / 3.f, halfMidX);
	slices[5]->setXStretch(1 / 3.f, halfMidX);
	slices[8]->setXStretch(1 / 3.f, halfMidX);

	// Adjust transforms to y stretch
	float midYScale = 1 / 3.f * yStretch;
	float halfMidY = 1 / 6.f + midYScale / 2.f;
	slices[0]->setYStretch(1 / 3.f, -halfMidY);
	slices[1]->setYStretch(1 / 3.f, -halfMidY);
	slices[2]->setYStretch(1 / 3.f, -halfMidY);

	slices[3]->setYStretch(midYScale, 0.0f);
	slices[4]->setYStretch(midYScale, 0.0f);
	slices[5]->setYStretch(midYScale, 0.0f);

	slices[6]->setYStretch(1 / 3.f, halfMidY);
	slices[7]->setYStretch(1 / 3.f, halfMidY);
	slices[8]->setYStretch(1 / 3.f, halfMidY);

	// Compensate with base scale
	float widthSum = 1 / 3.f + midXScale + 1 / 3.f;
	float heightSum = 1 / 3.f + midYScale + 1 / 3.f;
	this->scale(1 / widthSum, 1 / heightSum, 1.0f);
}

void SlicedSprite::initSprite(GLuint program)
{
	for (int i = 0; i < 9; i++) {
		slices[i]->initSprite(program);
	}
}

void SlicedSprite::setTextureID(GLuint managerTexID)
{
	for (int i = 0; i < 9; i++) {
		slices[i]->setTextureID(managerTexID);
	}
}

void SlicedSprite::render(PROJECTION projection) const
{
	pushModelMatrix();
	for (int i = 0; i < 9; i++) {
		slices[i]->render(projection);
	}
	OpenGLSprite::popMatrix();
}

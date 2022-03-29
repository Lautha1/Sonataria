#include "Transformable.h"

#include "OpenGLSprite.h"

Transformable::Transformable() : mPosition(0, 0, 0), mScale(1, 1, 1), mShear(0), mRotation(0, 0, 0), mPivot(0, 0, 0)
{
	// Initialize the two projection matrices
	mOrtho.setOrthoFrustum(-16.f / 9.f, 16.f / 9.f, -1.f, 1.f, -1.f, 1.f);
	mPersp.setPerspFrustum(60.f, 16.f / 9.f, 0.1f, 100.f);

	// Initialize all original transform props
	saveState();
}

void Transformable::updateTransformation()
{
	// Clear to identity
	mT.identity();

	// Translate so that the pivot point becomes zero
	mT.translate(-mPivot);

	// Apply scale
	mT.scale(mScale.x, mScale.y, mScale.z);

	// Apply Shear
	mT.shear(mShear);

	// Apply rotation
	mT.rotateX(mRotation.x);
	mT.rotateY(mRotation.y);
	mT.rotateZ(mRotation.z);

	// Put the pivot back
	mT.translate(mPivot);

	// Then apply the position
	mT.translate(mPosition);
}

void Transformable::translate(float dX, float dY, float dZ)
{
	mPosition.x += dX;
	mPosition.y += dY;
	mPosition.z += dZ;
	updateTransformation();
}

void Transformable::rotate(float rX, float rY, float rZ)
{
	mRotation.x = (mRotation.x + rX);
	mRotation.y = (mRotation.y + rY);
	mRotation.z = (mRotation.z + rZ);
	updateTransformation();
}

void Transformable::scale(float sX, float sY, float sZ)
{
	mScale.x *= sX;
	mScale.y *= sY;
	mScale.z *= sZ;
	updateTransformation();
}

void Transformable::setTranslate(float x, float y, float z)
{
	mPosition.x = x;
	mPosition.y = y;
	mPosition.z = z;
	updateTransformation();
}

void Transformable::setRotate(float x, float y, float z)
{
	mRotation.x = x;
	mRotation.y = y;
	mRotation.z = z;
	updateTransformation();
}

void Transformable::setScale(float x, float y, float z)
{
	mScale.x = x;
	mScale.y = y;
	mScale.z = z;
	updateTransformation();
}

void Transformable::shear(float value)
{
	mShear += value;
	updateTransformation();
}

void Transformable::setPivot(float x, float y, float z) {
	mPivot.x = x;
	mPivot.y = y;
	mPivot.z = z;
	updateTransformation();
}

void Transformable::saveState(bool savePosition, bool saveScale, bool saveRotation, bool savePivot)
{
	if (savePosition) { origPosition = mPosition; }
	if (saveScale) { origScale = mScale; }
	if (saveRotation) { origRotation = mRotation; }
	if (savePivot) { origPivot = mPivot; }
}

void Transformable::reset()
{
	mPosition = origPosition;
	mScale = origScale;
	mRotation = origRotation;
	mPivot = origPivot;
	updateTransformation();
}

void Transformable::makeIdentity()
{
	mPosition.set(0, 0, 0);
	mScale.set(1, 1, 1);
	mRotation.set(0, 0, 0);
	mPivot.set(0, 0, 0);
	updateTransformation();
}

void Transformable::setPosition(float newX, float newY) {
	float right = 1.f;
	float bottom = 9.f / 16.f;
	float screenWidth = 1920.f;
	float screenHeight = 1080.f;

	float newXConverted = (2.f * right) / screenWidth * newX - right;
	float newYConverted = (2.f * bottom) / screenHeight * newY - bottom;

	mPosition.x = newXConverted;
	mPosition.y = newYConverted;
	updateTransformation();
}

void Transformable::setPositionY(float newY) {
	float bottom = 9.f / 16.f;
	float screenHeight = 1080.f;

	float newYConverted = (2.f * bottom) / screenHeight * newY - bottom;

	mPosition.y = newYConverted;
	updateTransformation();
}

void Transformable::pushModelMatrix() const
{
	OpenGLSprite::pushMatrix(mT);
}

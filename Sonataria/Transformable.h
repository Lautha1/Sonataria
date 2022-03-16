#pragma once

#include "Matrices.h"

/// Simple Enum for selecting type of projection
enum PROJECTION {
	ORTHOGRAPHIC,
	PERSPECTIVE
};

/**
 * Class for any object that can be positioned, rotated and
 * scalled using a transformation matrix.
 */
class Transformable
{
public:
	Transformable();
	virtual ~Transformable() {}

	void translate(float dX, float dY, float dZ);
	void rotate(float rX, float rY, float rZ);
	void scale(float sX, float sY, float sZ);
	void shear(float value);
	inline void scale(float s) { scale(s, s, s); }

	void setPivot(float x, float y, float z);
	void setPosition(float newX, float newY);
	void setPositionY(float newY);

	void reset();
	void saveState(bool savePosition = true, bool saveScale = true, bool saveRotation = true, bool savePivot = true);
	void pushModelMatrix() const;
	void makeIdentity();

protected:
	// Transformation data
	Vector3 mPosition, mRotation, mScale, mPivot;
	Vector3 origPosition, origRotation, origScale, origPivot;
	Matrix4 mT, mOrtho, mPersp;
	float mShear;

	// Recompute the transformation matrix
	void updateTransformation();
};

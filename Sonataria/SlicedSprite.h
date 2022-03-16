#pragma once

#include <GL/glew.h>
#include "Transformable.h"
#include <string>

class QuadSprite;

class SlicedSprite : public Transformable
{
public:
	SlicedSprite(const std::wstring& newName, float xStretch = 1.0f, float yStretch = 1.0f);
	virtual ~SlicedSprite();

	void initSprite(GLuint program);
	virtual void render(PROJECTION projection) const;

	void setStretch(float xStretch = 1.0f, float yStretch = 1.0f);
	void setTextureID(GLuint managerTexID);

protected:
	QuadSprite* slices[9];
};

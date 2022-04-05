#pragma once
#include "OpenGLSprite.h"
#include "Animatable.h"

// Full vertex data (for easier initialization)
struct QuadVertData {
	Vector3 pos, color;
	Vector2 tex;
};

class QuadSprite : public OpenGLSprite, public Animatable
{
public:
	QuadSprite(const std::wstring& newName);
	QuadSprite(const std::wstring& newName, const QuadVertData* points, int newVertCount);
	virtual ~QuadSprite();

	void setVertexLocation(int index, float x, float y, float z = 0.0f);
	void setVertexPixelLocation(int index, float x, float y, float z = 0.0f);
	void scaleUVs(float uScale = 1.0f, float vScale = 1.0f);
	void offsetUVs(float uScale = 0.0f, float vScale = 0.0f);

	virtual void render(PROJECTION projType) const;

	QuadVertData* structPoints;

protected:
	void init();

	bool applyExtended(AnimationData& anim, int64_t currentSongOffset) override;
};

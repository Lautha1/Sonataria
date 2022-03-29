#pragma once

#include <vector>

#include "Transformable.h"

enum InterpolationType
{
	MOTION,
	LINEAR,
	CUBIC,
	COSINE,
	BOUNCE,
	EASE_IN,
	EASE_OUT
};

enum PropertyType
{
	POSITION,
	ROTATION,
	SCALE
};

struct AnimationData
{
	InterpolationType type;
	PropertyType prop;
	Vector3 startOrDelta, end;
	float durationRate;
	int64_t beginTime;
};

class Animatable : public Transformable
{
public:
	Animatable();
	~Animatable();

	void addMotion(PropertyType prop, Vector3 delta, float rate);
	void addInterpolation(InterpolationType interpType, PropertyType prop,
		Vector3 start, Vector3 end, float durationMsecs);

	void update(int64_t currentSongOffset);

private:
	std::vector<AnimationData> anims;

	void applyMotion(AnimationData &anim);
	void applyLinear(AnimationData &anim, int64_t currentSongOffset);
};

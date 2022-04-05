#pragma once

#include <vector>

#include "Transformable.h"

enum InterpolationType
{
	INTERP_MOTION,
	INTERP_LINEAR,
	INTERP_CUBIC,
	INTERP_COSINE,
	INTERP_BOUNCE,
	INTERP_EASE_IN,
	INTERP_EASE_OUT
};

enum PropertyType
{
	PROP_POSITION,
	PROP_ROTATION,
	PROP_SCALE,

	// Here and below are special properties
	PROP_EXTENDED,
	PROP_OPACITY
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
	void addInterpolation(InterpolationType interpType, PropertyType prop,
		float start, float end, float durationMsecs);

	void update(int64_t currentSongOffset);

protected:
	std::vector<AnimationData> anims;

	void applyMotion(AnimationData &anim);
	bool applyLinear(AnimationData &anim, int64_t currentSongOffset);

	// Override in child to animate properties besides transformations
	virtual bool applyExtended(AnimationData &anim, int64_t currentSongOffset) = 0;
};

#include "Animatable.h"

#include "Logger.h"

Animatable::Animatable() {}
Animatable::~Animatable() {}

void Animatable::addMotion(PropertyType prop, Vector3 delta, float rate)
{
	AnimationData newMotion = {
		MOTION, prop, delta, Vector3(), rate, -1
	};

	anims.push_back(newMotion);
}

void Animatable::addInterpolation(InterpolationType interpType, PropertyType prop,
	Vector3 start, Vector3 end, float durationMsecs)
{
	AnimationData newMotion = {
		interpType, prop, start, end, durationMsecs, -1
	};

	anims.push_back(newMotion);
}

void Animatable::update(int64_t currentSongOffset)
{
	 for (auto curAnim: anims)
	 {
		 switch (curAnim.type)
		 {
			 case MOTION: applyMotion(curAnim); break;
			 case LINEAR: applyLinear(curAnim, currentSongOffset); break;
			 default: break;
		 }
	 }
}

void Animatable::applyMotion(AnimationData &anim)
{
	switch(anim.prop)
	{
		case POSITION:
			this->translate(
				anim.startOrDelta.x * anim.durationRate,
				anim.startOrDelta.y * anim.durationRate,
				anim.startOrDelta.z * anim.durationRate
			);
			break;

		case ROTATION:
			this->rotate(
				anim.startOrDelta.x * anim.durationRate,
				anim.startOrDelta.y * anim.durationRate,
				anim.startOrDelta.z * anim.durationRate
			);
			break;

		case SCALE:
			this->scale(
				anim.startOrDelta.x * anim.durationRate,
				anim.startOrDelta.y * anim.durationRate,
				anim.startOrDelta.z * anim.durationRate
			);
			break;

		default:
			logger.logError("Unknown property in animation");
			break;
	}
}

void Animatable::applyLinear(AnimationData &anim, int64_t currentSongOffset)
{
	// Set begin time if needed
	if (anim.beginTime < 0)
	{
		anim.beginTime = currentSongOffset;
	}

	// Compute 'alpha'
	int64_t elapsed = currentSongOffset - anim.beginTime;
	float alpha = elapsed / anim.durationRate;

	// Do interpolation
	Vector3 currentValue = Vector3(
		anim.startOrDelta.x * (1.0f - alpha) + anim.end.x * alpha,
		anim.startOrDelta.y * (1.0f - alpha) + anim.end.y * alpha,
		anim.startOrDelta.z * (1.0f - alpha) + anim.end.z * alpha
	);

	switch (anim.prop)
	{
		case POSITION:
			this->setTranslate(currentValue.x, currentValue.y, currentValue.z);
			break;

		case ROTATION:
			this->setRotate(currentValue.x, currentValue.y, currentValue.z);
			break;

		case SCALE:
			this->setScale(currentValue.x, currentValue.y, currentValue.z);
			break;

		default:
			logger.logError("Unknown property in animation");
			break;
	}
}

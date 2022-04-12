#include "Animatable.h"

#include "Logger.h"

using namespace std;

Animatable::Animatable() {}
Animatable::~Animatable() {}

void Animatable::addMotion(PropertyType prop, Vector3 delta, float rate)
{
	AnimationData newMotion = {
		INTERP_MOTION, prop, delta, Vector3(), rate, -1
	};

	anims.push_back(newMotion);
}

void Animatable::addInterpolation(InterpolationType interpType, PropertyType prop,
	float start, float end, float durationMsecs)
{
	addInterpolation(interpType, prop,
		Vector3(start, 0.0f, 0.0f),
		Vector3(end, 0.0f, 0.0f),
		durationMsecs);
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
	vector<size_t> toRemove;
	for (size_t i = 0; i < anims.size(); i++)
	{
		// Set begin time if needed
		if (anims[i].beginTime < 0LL)
		{
			anims[i].beginTime = currentSongOffset;
		}

		if (anims[i].prop > PROP_EXTENDED)
		{
			if (!applyExtended(anims[i], currentSongOffset))
			{
				toRemove.push_back(i);
			}
		}
		else
		{
			// Defer based on motion type
			switch (anims[i].type)
			{
				case INTERP_MOTION: applyMotion(anims[i]); break;
				case INTERP_LINEAR: {
					bool keepGoing = applyLinear(anims[i], currentSongOffset);
					if (!keepGoing) { toRemove.push_back(i); }
				} break;

				default: break;
			}
		}
	}

	for (auto remove: toRemove)
	{
		anims.erase(anims.begin() + remove);
	}
}

void Animatable::applyMotion(AnimationData &anim)
{
	switch(anim.prop)
	{
		case PROP_POSITION:
			this->translate(
				anim.startOrDelta.x * anim.durationRate,
				anim.startOrDelta.y * anim.durationRate,
				anim.startOrDelta.z * anim.durationRate
			);
			break;

		case PROP_ROTATION:
			this->rotate(
				anim.startOrDelta.x * anim.durationRate,
				anim.startOrDelta.y * anim.durationRate,
				anim.startOrDelta.z * anim.durationRate
			);
			break;

		case PROP_SCALE:
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

bool Animatable::applyLinear(AnimationData &anim, int64_t currentSongOffset)
{
	// Compute 'alpha'
	int64_t elapsed = currentSongOffset - anim.beginTime;
	float alpha = elapsed / anim.durationRate;
	if (alpha >= 1.0)
	{
		return false;
	}

	// Do interpolation
	Vector3 currentValue = Vector3(
		anim.startOrDelta.x * (1.0f - alpha) + anim.end.x * alpha,
		anim.startOrDelta.y * (1.0f - alpha) + anim.end.y * alpha,
		anim.startOrDelta.z * (1.0f - alpha) + anim.end.z * alpha
	);

	switch (anim.prop)
	{
		case PROP_POSITION:
			this->setTranslate(currentValue.x, currentValue.y, currentValue.z);
			break;

		case PROP_ROTATION:
			this->setRotate(currentValue.x, currentValue.y, currentValue.z);
			break;

		case PROP_SCALE:
			this->setScale(currentValue.x, currentValue.y, currentValue.z);
			break;

		default:
			logger.logError("Unknown property in animation");
			break;
	}

	return true;
}

#include "Animatable.h"

Animatable::Animatable() {}
Animatable::~Animatable() {}

void Animatable::addMotion(PropertyType prop, Vector3 delta, double rate)
{
	AnimationData newMotion = {
		MOTION, prop, delta, Vector3(), rate
	};

	anims.push_back(newMotion);
}

void Animatable::addInterpolation(InterpolationType interpType, PropertyType prop,
	Vector3 start, Vector3 end, double durationMsecs)
{
	AnimationData newMotion = {
		interpType, prop, start, end, durationMsecs
	};

	anims.push_back(newMotion);
}

void Animatable::update(int64_t currentSongOffset)
{
	 for (const auto curAnim: anims)
	 {
		 switch (curAnim.type)
		 {
			 case MOTION: applyMotion(curAnim); break;
			 case LINEAR: applyLinear(curAnim); break;
			 default: break;
		 }
	 }
}

void Animatable::applyMotion(AnimationData anim)
{
	
}

void Animatable::applyLinear(AnimationData anim)
{

}

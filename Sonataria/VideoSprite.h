#pragma once
#include "QuadSprite.h"

class AVDecode;

class VideoSprite : public QuadSprite
{
public:
	VideoSprite(const std::wstring& newName, bool enableLoop = false);
	virtual ~VideoSprite();

	void setStartTimeInSeconds(double startTime);
	void enableLooping(bool newEnableLoop);
	bool loadVideo(const char* videoFilename);
	void update(double elapsedTimeSec);
	virtual void render(PROJECTION projType) const;

protected:
	// The video decoder object
	AVDecode* myDecoder;

	// Playback state
	bool videoDone, enableLoop;
	double curFrameTimestampSec;
	double loopOffset, startOffset;

	// Internal texture IDs
	GLuint textures[3];
};

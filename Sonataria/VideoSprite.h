#pragma once
#include "QuadSprite.h"

class AVDecode;

class VideoSprite : public QuadSprite
{
public:
	VideoSprite(const std::wstring& newName);
	virtual ~VideoSprite();

	bool loadVideo(const char* videoFilename);
	void update(double elapsedTimeSec);
	virtual void render(PROJECTION projType) const;

protected:
	// The video decoder object
	AVDecode* myDecoder;

	// Playback state
	bool videoDone;
	double curFrameTimestampSec;

	// Internal texture IDs
	GLuint textures[3];
};

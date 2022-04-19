#include "VideoSprite.h"

#include <string>
#include "Logger.h"
#include "AVDecode.h"
using namespace std;

VideoSprite::VideoSprite(const std::wstring& newName, bool enableLoop) : QuadSprite(newName)
{
	myDecoder = new AVDecode();
	videoDone = true; // Until the video is loaded, say it is done
    this->enableLoop = enableLoop;
    loopOffset = startOffset = 0.0f;
}

VideoSprite::~VideoSprite()
{
    delete myDecoder;
    glDeleteTextures(3, textures);
}

void VideoSprite::setStartTimeInSeconds(double startTime)
{
	startOffset = startTime;
}

void VideoSprite::enableLooping(bool newEnableLoop)
{
    enableLoop = newEnableLoop;
}

bool VideoSprite::loadVideo(const char* videoFilename)
{
	// Attempt to initialize the video decoder
	if (!myDecoder->prepareToDecode(videoFilename))
	{
		return false;
	}
    videoDone = false;

    // Make the textures that will hold the video frame color planes
    const int width = myDecoder->getWidth();
    const int height = myDecoder->getHeight();

    // Generate the textures in OpenGL state only
    glGenTextures(3, textures);
    glBindTexture(GL_TEXTURE_2D, textures[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_2D, textures[1]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, width / 2, height / 2, 0, GL_RED, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_2D, textures[2]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, width / 2, height / 2, 0, GL_RED, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Update the decoder textures
    myDecoder->setTextureIDs(this->textures[0], this->textures[1], this->textures[2]);

    // Return success
	return true;
}

void VideoSprite::update(double elapsedTimeSec)
{
    if (!videoDone && (elapsedTimeSec - loopOffset) >= curFrameTimestampSec + startOffset) {
        // Skip a frame after rewinding to avoid timestamp issue
        if (curFrameTimestampSec <= 0) { myDecoder->decodeFrame(); }

        // Decode next frame
    	if ((curFrameTimestampSec = myDecoder->decodeFrame()) < 0) {
            if (enableLoop)
            {
	            myDecoder->rewind();
                loopOffset = elapsedTimeSec;
            }
            else
            {
	            myDecoder->closeDecoder();
	            videoDone = true;
            }
        }
    }
}

void VideoSprite::render(PROJECTION projType) const
{
    // Bind the three texture planes to the first three texture targets
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textures[0]);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, textures[1]);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, textures[2]);

    // Render the underlying quad
    QuadSprite::render(projType);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, 0);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, 0);
}

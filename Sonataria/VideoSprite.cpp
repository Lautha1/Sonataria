#include "VideoSprite.h"

#include "AVDecode.h"

VideoSprite::VideoSprite(const std::wstring& newName) : QuadSprite(newName)
{
	myDecoder = new AVDecode();
	videoDone = true; // Until the video is loaded, say it is done
}

VideoSprite::~VideoSprite()
{
    delete myDecoder;
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
    glGenTextures(1, &this->textures[0]);
    glBindTexture(GL_TEXTURE_2D, this->textures[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glGenTextures(1, &this->textures[1]);
    glBindTexture(GL_TEXTURE_2D, this->textures[1]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, width / 2, height / 2, 0, GL_RED, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glGenTextures(1, &this->textures[2]);
    glBindTexture(GL_TEXTURE_2D, this->textures[2]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, width / 2, height / 2, 0, GL_RED, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Update the decoder textures
    myDecoder->setTextureIDs(this->textures[0], this->textures[1], this->textures[2]);

    // Return success
	return true;
}

void VideoSprite::update(int elapsedTimeMillis)
{
    // Attempt to update the video frame
    if (!videoDone) {
        if (!myDecoder->decodeFrame()) {
            myDecoder->closeDecoder();
            videoDone = true;
        }
    }
}

void VideoSprite::render(PROJECTION projType) const
{
    // Bind the three texture planes to the first three texture targets
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, this->textures[0]);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, this->textures[1]);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, this->textures[2]);

    // Render the underlying quad
    QuadSprite::render(projType);
}

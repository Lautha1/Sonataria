#include "MusicPlayer.h"
#include "Logger.h"

MusicPlayer musicPlayer;

MusicPlayer::MusicPlayer() {}

MusicPlayer::~MusicPlayer() {}

void MusicPlayer::playSong(std::string pathToFile) {
	// Don't start playing if already playing
	if (pathToFile == this->currentSong) {
		return;
	}

	// Stop music 
	this->music.stop();

	// Load new song
	if (!this->music.openFromFile(pathToFile)) {
		logger.logError("Failed to open music: ", pathToFile);
	}

	// Set Looping
	this->music.setLoop(true);

	// Play song
	this->music.play();

	this->currentSong = pathToFile;
}

void MusicPlayer::stopSong() {
	// Stop music 
	this->music.stop();
	this->currentSong = "";
}
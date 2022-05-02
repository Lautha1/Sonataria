#include "SFML/Audio.hpp"
#include <string>
#pragma once

class MusicPlayer {

	public:
		MusicPlayer();
		~MusicPlayer();
		void playSong(std::string pathToFile);
		void stopSong();

	private:
		sf::Music music;
		std::string currentSong;
};

extern MusicPlayer musicPlayer;
#include "SFML/Audio.hpp"
#pragma once
class SoundEffects {
	public:
		enum Effects {
			FX_CardTap,
			FX_Error
		};

		SoundEffects();
		~SoundEffects();
		void playSoundEffect(Effects sfx);

	private:
		// All SFX get their own buffer
		sf::SoundBuffer CardTap;
		sf::SoundBuffer Error;

		// The actual object that plays the sounds
		sf::Sound player;
};

extern SoundEffects soundEffects;
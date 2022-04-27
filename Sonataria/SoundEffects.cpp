#include "Logger.h"
#include "SoundEffects.h"
using namespace std;

SoundEffects soundEffects;

SoundEffects::SoundEffects() {
	CardTap.loadFromFile("MusicFX/CardTap.wav");
}

SoundEffects::~SoundEffects() {}

void SoundEffects::playSoundEffect(Effects sfx) {
	// Set the sound buffer to play
	switch (sfx) {
		case Effects::FX_CardTap:
			player.setBuffer(CardTap);
			break;
	}
	
	// Play from the buffer
	logger.log("Playing sound effect: ", to_string(sfx));
	player.play();
}

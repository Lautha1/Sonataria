#include "GameState.h"
#include <iostream>
#include "RFIDCardReader.h"
#include "ScreenRenderer.h"
#include "UserData.h"

GameState gameState;

/**
 * Default constructor.
 * 
 */
GameState::GameState() {
	// Initialize all startup defaults
	this->state = CurrentState::STARTUP;
	this->onlineState = OnlineState::OFFLINE;
	this->err_code = 0;
	this->servicePressed = false;
	this->difficulty = 0;
	this->speed = 0;

	// Value in Milliseconds
	this->CurtainTransitionTime = 2500.f;
}

/**
 * Default deconstructor.
 * 
 */
GameState::~GameState() {

}

/**
 * Get the current gamestate the game is in.
 * 
 * @return the game state (as a string)
 */
GameState::CurrentState GameState::getGameState() {
	return this->state;
}

/**
 * Sets the game state to a new provided state.
 * 
 * @param newState the state to set the game state to
 */
void GameState::setGameState(GameState::CurrentState newState) {
	this->onStateUnload(this->state);
	// DELAY THIS
	this->state = newState;
	this->onStateLoad(newState);
}

/**
 * Logic to handle when states are unloaded.
 *
 * @param stateLoaded the state being unloaded
 */
void GameState::onStateUnload(GameState::CurrentState stateUnloaded) {
	// Close Curtains
	screenRenderer.ToggleCurtains(false);

	// Handle Any Between Scene Items
	switch (stateUnloaded) {
		default:
			//Do Nothing
			break;
	}
}

/**
 * Logic to handle when states are loaded.
 * 
 * @param stateLoaded the state being loaded
 */
void GameState::onStateLoad(GameState::CurrentState stateLoaded) {
	// Open Curtains
	screenRenderer.ToggleCurtains(true);

	// Handle Any Scene Load Items
	switch (stateLoaded) {
		default:
		case GameState::CurrentState::TITLE_SCREEN:
			resetResults();
			userData.clearData();
			break;
		case GameState::CurrentState::PRELOGIN:
			RFIDCardReader::getCardReader()->clearLastCardData();
			break;
	}
}

/**
 * Set the game state to the error state with an error code.
 * 
 * @param errorCode the error code to set the game state to
 */
void GameState::setGameState(int errorCode) {
	state = CurrentState::ERROR_CODE;
	err_code = errorCode;
}

/**
 * Set the state of the service button being pressed.
 * 
 * @param newServ new service button state
 */
void GameState::setServicePressed(bool newServ) {
	servicePressed = newServ;
}

/**
 * Return the state of the service button.
 * 
 * @return service button state
 */
bool GameState::checkService() {
	return servicePressed;
}

/**
 * Set the song and difficulty currently being played.
 * 
 * @param newSong the song being played
 * @param diff the difficulty of the song
 */
void GameState::setSongPlaying(Song newSong, int diff) {
	this->currentlyPlaying = newSong;
	this->difficulty = diff;
}

/**
 * Get the song that is currently playing.
 * 
 * @return the currently playing song
 */
Song GameState::getSongPlaying() {
	return this->currentlyPlaying;
}

/**
 * Get the difficulty of the song that is currently playing.
 * 
 * @return the difficulty of the song
 */
int GameState::getSongPlayingDifficulty() {
	return this->difficulty;
}

/**
 * Reset the vector containing that sessions results.
 * 
 */
void GameState::resetResults() {
	// If the vector was already empty
	// Don't do anything
	if (this->results.empty()) {
		return;
	}
	else { // Clear the array
		this->results.clear();
	}
}

/**
 * Set the speed that the player is playing at.
 * 
 * @param newSpeed the new speed
 */
void GameState::setSpeed(int newSpeed) {
	this->speed = newSpeed;
}

/**
 * Get the speed the player is playing at.
 * 
 * @return the current speed
 */
int GameState::getSpeed() {
	return this->speed;
}

/**
 * Set the new online state.
 * 
 * @param newState the state to be set
 */
void GameState::setOnlineState(OnlineState newState) {
	this->onlineState = newState;
}

/**
 * Get the current online state.
 * 
 * @return the online state
 */
GameState::OnlineState GameState::getOnlineState() {
	return this->onlineState;
}

/**
 * Get the online state as a string.
 * 
 * @return string version of online state
 */
string GameState::getOnlineStateStr() {
	switch (this->onlineState) {
		case OnlineState::MAINTENENCE:
			return "MAINTENENCE";
		case OnlineState::OFFLINE:
			return "OFFLINE";
		case OnlineState::ONLINE:
			return "ONLINE";
		default:
			return "";
	}
}
#include <chrono>
#include "GameState.h"
#include <iostream>
#include "Logger.h"
#include "RFIDCardReader.h"
#include "ScreenRenderer.h"
#include <thread>
#include "UserData.h"

GameState gameState;

// Forward Declarations
void CurtainDelay(int, bool);
void GameStateChangeThread(GameState::CurrentState, GameState::CurrentState);

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
	this->CurtainTransitionTime = 1500;
	this->isTransitioning = false;

	this->isSwitchingStates = false;
}

/**
 * Default deconstructor.
 * 
 */
GameState::~GameState() {}

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
	if (this->isSwitchingStates) { return; }

	// Don't allow another swap state while currently switching states
	this->isSwitchingStates = true;

	// Prevent "T" presses while swapping game states
	this->isTransitioning = true;

	// Launch a thread to prevent blocking
	thread changeState;
	try {
		changeState = std::thread(GameStateChangeThread, this->state, newState);
		changeState.detach();
	}
	catch (exception e) {
		logger.logError(e.what());
	}

	// We don't unlock the "T" key here
	// It is unlocked in the launched thread so that it ensures the game state finished swapping
}

void GameState::DoneSwitchingStates() {
	this->isSwitchingStates = false;
}

void GameStateChangeThread(GameState::CurrentState oldState, GameState::CurrentState newState) {
	// Close Curtains
	thread delay;
	if (!gameState.isInServiceGameState() && !gameState.isInServiceGameState(newState)) {
		// Begin Animation
		screenRenderer.ToggleCurtains(false);

		// Only add the delay if we actually transition
		delay = std::thread(CurtainDelay, gameState.CurtainTransitionTime, true);

		// Wait for it to finish
		delay.join();
	}

	// Unload the State
	gameState.onStateUnload(oldState);

	// Switch the State
	gameState.state = newState;

	// Handle loading the state
	gameState.onStateLoad(newState);

	// Open Curtains
	if (!gameState.isInServiceGameState() && !gameState.isInServiceGameState(newState)) {
		// Begin Animation
		screenRenderer.ToggleCurtains(true);

		// Add the delay again to make sure the transition ends
		delay = std::thread(CurtainDelay, gameState.CurtainTransitionTime, false);

		// Wait for it to finish
		delay.join();
	}

	// Allow "T" presses again
	gameState.isTransitioning = false;
	gameState.DoneSwitchingStates();
}

void CurtainDelay(int mSec, bool addExtraDelay) {
	int totalTime = mSec;

	// Add extra time on the curtain close before reopening
	if (addExtraDelay) { totalTime += 1000; }

	// Sleep the thread
	std::this_thread::sleep_for(std::chrono::milliseconds(totalTime));
}

/**
 * Logic to handle when states are unloaded.
 *
 * @param stateLoaded the state being unloaded
 */
void GameState::onStateUnload(GameState::CurrentState stateUnloaded) {
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

// Check if the current game state is one of the service states
bool GameState::isInServiceGameState() {
	for (size_t i = 0; i < this->serviceStates.size(); i++) {
		if (this->state == this->serviceStates[i]) {
			return true;
		}
	}
	return false;
}

// Check if a specific game state is one of the service states
bool GameState::isInServiceGameState(CurrentState stateToCheck) {
	for (size_t i = 0; i < this->serviceStates.size(); i++) {
		if (stateToCheck == this->serviceStates[i]) {
			return true;
		}
	}
	return false;
}
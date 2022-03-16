#include "ControllerInput.h"
#include "GameState.h"
#include <PacDrive/PacDrive.h>
#include "KeyboardState.h"
#include <iostream>

ControllerInput controllerInput;

/**
 * Default Constructor.
 * 
 */
ControllerInput::ControllerInput() {
	// Set wheel positions (mouse) to 0
	this->mousePos = 0;
	this->lastMousePos = 0;
}

/**
 * Default Deconstructor.
 * 
 */
ControllerInput::~ControllerInput() {

}

/**
 * Gets the keyboard state.
 * 
 * @return the keyboard state
 */
KeyboardState ControllerInput::getKeyboardState() {
	return keyboard;
}

/**
 * Set the state of a key to on/off.
 * 
 * @param keyNum the key to set the state of
 * @param state the state of the key (true-on | false-off)
 */
void ControllerInput::setKeyState(int keyNum, bool state) {
	
	this->keyboard.setKeyState(keyNum, state);

	// Only push to input queue if on the game screen and not the start button and only on button down
	if (gameState.getGameState() == GameState::CurrentState::GAME && keyNum != 6 && state == true) {
		this->queue_mutexLock.lock();
		this->inputQueue.push_back(keyNum);
		this->queue_mutexLock.unlock();
	}
}

/**
 * Reset all of the states of the keys.
 * 
 */
void ControllerInput::reset() {
	// Set all keys to false
	this->setKeyState(1, false);
	this->setKeyState(2, false);
	this->setKeyState(3, false);
	this->setKeyState(4, false);
	this->setKeyState(5, false);
	this->setKeyState(6, false);

	// Turn off the LED lights since if they keys
	// are no longer pressed, the lights shouldn't be on
	PacSetLEDStates(0, 0x0);

	// Reset the wheel (mouse) position to 0
	sf::Mouse::setPosition(sf::Vector2i(1, 0));
	setWheelPos(0);
}

/**
 * Update the position of the wheel.
 * 
 * @param change the amount to change the wheel position (can be positive or negative)
 */
void ControllerInput::changeWheelPos(int change) {
	this->lastMousePos = this->mousePos;
	this->mousePos += change;
	//TODO: Add check to make sure this doesn't over/underflow the int value
}

/**
 * Used to set the position of the wheel.
 * 
 * @param val the current position of the wheel
 */
void ControllerInput::setWheelPos(int val) {
	this->mousePos = val;
	this->lastMousePos = val;
}

/**
 * Get the position of the wheel.
 * 
 * @return the position of the wheel
 */
int ControllerInput::getWheelPos() {
	return this->mousePos;
}

/**
 * Determine and return the direction the wheel is moving.
 * 
 * @return 
 */
int ControllerInput::getWheelMovement() {
	if (this->mousePos > this->lastMousePos) { // Moving ->
		return 1;
	}
	else if (this->mousePos < this->lastMousePos) { // Moving <-
		return -1;
	}
	else { // Not Moved
		return 0;
	}
}

/**
 * Reset the last position state to be equal to the current position.
 * 
 */
void ControllerInput::resetLast() {
	this->lastMousePos = this->mousePos;
}

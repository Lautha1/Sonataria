#include "Key.h"

/**
 * Default constructor.
 * 
 */
Key::Key() {
	isCurrentlyDown = false;
}

/**
 * Default deconstructor.
 * 
 */
Key::~Key() {

}

/**
 * Returns the current state of a key.
 * 
 * @return key state (up or down)
 */
bool Key::checkPressed() {
	return isCurrentlyDown;
}

/**
 * Set the state of a key.
 * 
 * @param newState the new state of the key
 */
void Key::setState(bool newState) {
	isCurrentlyDown = newState;
}
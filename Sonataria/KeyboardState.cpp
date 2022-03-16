#include "KeyboardState.h"
using namespace std;

/**
 * Default constructor.
 * 
 */
KeyboardState::KeyboardState() {

}

/**
 * Default deconstructor.
 * 
 */
KeyboardState::~KeyboardState() {

}

/**
 * Set the state of a key.
 * 
 * @param keyNumber the key to set
 * @param isDown the state of the key
 */
void KeyboardState::setKeyState(int keyNumber, bool isDown) {
	switch (keyNumber) {
		case 1:
			btn1.setState(isDown);
			break;
		case 2:
			btn2.setState(isDown);
			break;
		case 3:
			btn3.setState(isDown);
			break;
		case 4:
			btn4.setState(isDown);
			break;
		case 5:
			btn5.setState(isDown);
			break;
		case 6:
			start6.setState(isDown);
			break;
	}
}

/**
 * Get state state of the key.
 * 
 * @param keyNumber the key
 * @return the state of the key
 */
bool KeyboardState::getKeyState(int keyNumber) {
	switch (keyNumber) {
		case 1:
			return btn1.checkPressed();
		case 2:
			return btn2.checkPressed();
		case 3:
			return btn3.checkPressed();
		case 4:
			return btn4.checkPressed();
		case 5:
			return btn5.checkPressed();
		case 6:
			return start6.checkPressed();
		default:
			return false;
	}
}
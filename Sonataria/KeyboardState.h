/**
 * @file KeyboardState.h
 *
 * @brief Keyboard State
 *
 * @author Julia Butenhoff
 */
#pragma once
#include "Key.h"
using namespace std;

/**
 * Holds the states of all buttons on the controller
 * 
 * It is referenced as keyboard because all of the buttons
 * on the controller are mapped to a key on the keyboard
 * btn1 -> A
 * btn2 -> C
 * btn3 -> B
 * btn4 -> M
 * btn5 -> L
 * start6 -> T
 */
class KeyboardState {

	private:
		Key btn1;
		Key btn2;
		Key btn3;
		Key btn4;
		Key btn5;
		Key start6;

	public:
		KeyboardState();
		~KeyboardState();
		void setKeyState(int, bool);
		bool getKeyState(int);
};
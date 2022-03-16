/**
 * @file ControllerInput.h
 *
 * @brief Controller Input
 *
 * @author Julia Butenhoff
 */
#pragma once
#include <deque>
#include "KeyboardState.h"
#include <mutex>
#include <SFML/Graphics.hpp>

/**
 * Handles all of the input from the controller
 */
class ControllerInput {

	private:
		KeyboardState keyboard;
		int mousePos;
		int lastMousePos;

	public:
		ControllerInput();
		~ControllerInput();
		KeyboardState getKeyboardState();
		void setKeyState(int, bool);
		void reset();
		void changeWheelPos(int);
		void setWheelPos(int);
		int getWheelPos();
		deque<int> inputQueue;
		mutex queue_mutexLock;
		int getWheelMovement();
		void resetLast();
};

extern ControllerInput controllerInput;
/**
 * @file Key.h
 *
 * @brief Key
 *
 * @author Julia Butenhoff
 */
#pragma once
using namespace std;

/**
 * Holds the state of an individual button
 */
class Key {

	private:
		bool isCurrentlyDown;

	public:
		Key();
		~Key();
		bool checkPressed();
		void setState(bool);
};
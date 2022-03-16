/**
 * @file Results.h
 *
 * @brief Results
 *
 * @author Julia Butenhoff
 */
#pragma once
#include "Song.h"
#include <string>
using namespace std;

/**
 * Stores the results data from a song
 */
class Results {

	private:
		Song song;
		int difficulty;
		int score;
		int perfectCount;
		int nearCount;
		int missCount;

	public:
		Results(Song, int, int, int, int, int);
		~Results();
		Song getSong();
		int getDifficulty();
		int getScore();
		int getPerfectCount();
		int getNearCount();
		int getMissCount();
};
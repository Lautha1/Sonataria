#include "Results.h"

/**
 * Constructor for a resuls object.
 * 
 * @param s The Song Object
 * @param d The Difficulty Played
 * @param sc The Score
 * @param pCount The Count of Perfects
 * @param nCount The Count of Nears
 * @param mCount The Count of Misses
 */
Results::Results(Song s, int d, int sc, int pCount, int nCount, int mCount) {
	this->song = s;
	this->difficulty = d;
	this->score = sc;
	this->perfectCount = pCount;
	this->nearCount = nCount;
	this->missCount = mCount;
}

/**
 * Default Deconstructor.
 * 
 */
Results::~Results() {

}

/**
 * Get the song from the results.
 * 
 * @return the song
 */
Song Results::getSong() {
	return this->song;
}

/**
 * Get the difficulty from the results.
 * 
 * @return the difficulty
 */
int Results::getDifficulty() {
	return this->difficulty;
}

/**
 * Get the score from the results.
 * 
 * @return the score
 */
int Results::getScore() {
	return this->score;
}

/**
 * Get the number of perfects.
 * 
 * @return the number of perfects
 */
int Results::getPerfectCount() {
	return this->perfectCount;
}

/**
 * Get the number of nears.
 * 
 * @return the number of nears
 */
int Results::getNearCount() {
	return this->nearCount;
}

/**
 * Get the number of misses.
 * 
 * @return the number of misses
 */
int Results::getMissCount() {
	return this->missCount;
}
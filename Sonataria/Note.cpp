#include <cmath>
#include "Note.h"

/**
 * Constructor for a note.
 * 
 * @param t -> time of a hote hit perfect
 * @param isH -> is a hold?
 * @param hDur -> duration of the hold
 * @param nDen -> note density
 * @param bpm -> bpm of the song
 */
Note::Note(float t, bool isH, float hDur, int nDen, int bpm) {
	this->perfectTime = t;
	this->is_hold = isH;
	this->holdLength = hDur;
	this->noteDensity = nDen;

	this->endTime = calculateEndTime();

	this->appearTime = 0.f;
	speedChangeAppearTime(bpm);

	this->hitHold = false;

	this->notesRemaining = this->noteDensity;
	this->holdNoteDistance = (float)this->holdLength / ((float)this->noteDensity - 1.f);
	this->notesUsed = 0;
}

/**
 * Default deconstructor.
 * 
 */
Note::~Note() {

}

/**
 * Gets if the note is a hold note.
 * 
 * @return true if a hold note
 */
bool Note::isHold() {
	return this->is_hold;
}

/**
 * Get the amount of notes a hold is worth.
 * 
 * @return the number of notes
 */
int Note::getHoldNoteQuantity() {
	return this->noteDensity;
}

/**
 * Set when the note appears based on the bpm.
 * 
 * @param bpm the bpm of the song
 */
void Note::speedChangeAppearTime(int bpm) {
	float distance = (float)(453075.9705 / pow(bpm, 1.004140998));
	this->appearTime = this->perfectTime - distance;
}

/**
 * Not used yet.
 * 
 */
void Note::speedChangePosition() {
	// TODO: something?
}

/**
 * Calculate when the hold is complete.
 * 
 * @return when the hold is done
 */
float Note::calculateEndTime() {
	return this->perfectTime + this->holdLength;
}

/**
 * Get when the note ends.
 * 
 * @return end time of the note
 */
float Note::getEndTime() {
	return this->endTime;
}

/**
 * Gets how long the hold is.
 * 
 * @return the length of the hold
 */
float Note::getHoldLength() {
	return this->holdLength;
}

/**
 * Gets if you hit the hold note.
 * 
 * @return true if hit hold
 */
bool Note::didHitHold() {
	return this->hitHold;
}

/**
 * Set if the hold was hit.
 * 
 * @param state new state of the hold
 */
void Note::setHitHold(bool state) {
	this->hitHold = state;
}

/**
 * Subtract one from the amount of notes remaining on the hold.
 * 
 */
void Note::subtractHold() {
	this->notesRemaining -= 1;
	this->notesUsed += 1;
}

/**
 * Get the distance of a hold note.
 * 
 * @return the hold note distance
 */
float Note::getHoldNoteDistance() {
	return this->holdNoteDistance;
}

/**
 * Get the number of hold notes done.
 * 
 * @return the number of hold notes done
 */
int Note::getHoldNotesUsed() {
	return this->notesUsed;
}

/**
 * Get the number of hold notes left.
 * 
 * @return the number of hold notes left
 */
int Note::getHoldNotesRemaining() {
	return this->notesRemaining;
}

/**
 * Returns the float y position on screen
 * 0.f being top of screen
 * 1080.f being bottom
 * 
 * @return the float y position on screen
 */
float Note::getYPos() {
	return (1080.f - this->yPos);
}

/**
 * Sets the note's current y position on screen to the set value.
 * 
 * @param newYPos float of y position calculated on screen
 */
void Note::setYPos(float newYPos) {
	this->yPos = newYPos;
}

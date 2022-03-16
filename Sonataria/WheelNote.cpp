#include <cmath>
#include "WheelNote.h"

/**
 * Constructor for a wheel note.
 * 
 * @param t -> time (in milliseconds)
 * @param isS -> is_slam (true or false)
 * @param dir -> direction (1 - right | -1 - left)
 * @param sPos -> startPos (0 if doesn't apply)
 * @param ePos -> endPos (0 if doesn't apply)
 * @param l -> length (0 if slam)
 * @param nDen -> noteDensity (1 if slam)
 * @param bpm the bpm of the song
 */
WheelNote::WheelNote(float t, bool isS, int dir, int sPos, int ePos, float l, int nDen, int bpm) {
	this->perfectTime = t;
	this->is_slam = isS;
	this->direction = dir;
	this->startPos = sPos;
	this->endPos = ePos;
	this->length = l;
	this->noteDensity = nDen;

	this->endTime = calculateEndTime();

	this->appearTime = 0.f;
	speedChangeAppearTime(bpm);

	if (!isS) {
		this->calcWheelPos();
		this->calcWheelSize();
	}
	else {
		this->wheelPos = 0;
	}

	this->notesRemaining = this->noteDensity;
	this->holdNoteDistance = (float)this->length / ((float)this->noteDensity - 1.f);
	this->notesUsed = 0;
}

int WheelNote::getStartPos() const {
	return startPos;
}

int WheelNote::getEndPos() const {
	return endPos;
}

/**
 * Get the end time of the wheel note.
 * 
 * @return the end time of the wheel note
 */
float WheelNote::getEndTime() {
	return this->endTime;
}

/**
 * Calculate the end time of the wheel note.
 * 
 * @return the end time of the wheel note
 */
float WheelNote::calculateEndTime() {
	return this->perfectTime + this->length;
}

/**
 * Default deconstructor.
 * 
 */
WheelNote::~WheelNote() {

}

/**
 * Get if the wheel note is a slam.
 * 
 * @return true if a slam
 */
bool WheelNote::isSlam() {
	return this->is_slam;
}

/**
 * Get the density of the wheel note.
 * 
 * @return the density of the wheel note
 */
int WheelNote::getNoteQuantity() {
	return this->noteDensity;
}

/**
 * Set the appear time of the note.
 * 
 * @param bpm the bpm of the song
 */
void WheelNote::speedChangeAppearTime(int bpm) {
	float distance = (float)(453075.9705 / pow(bpm, 1.004140998));
	this->appearTime = this->perfectTime - distance;
}

/**
 * Get the direction of the wheel note.
 * 
 * @return the direction
 */
int WheelNote::getDirection() {
	return this->direction;
}

/**
 * Get the length that wheel goes across lanes to the right.
 * 
 * @return number of lanes moved
 */
int WheelNote::getCrossLengthRight() {
	return this->endPos - this->startPos;
}

/**
 * Get the length that wheel goes across lanes to the left.
 * 
 * @return number of lanes moved
 */
int WheelNote::getCrossLengthLeft() {
	return this->startPos - this->endPos;
}

/**
 * Get the length of the wheel note.
 * 
 * @return wheel note length
 */
float WheelNote::getWheelNoteLength() {
	return this->length;
}

/**
 * Set where to draw the position of the wheel between two lanes.
 * 
 */
void WheelNote::calcWheelPos() {
	if (this->direction == 1) { // ->
		if (this->startPos == 1) {
			if (this->endPos == 2) {
				this->wheelPos = 1.5;
			}
			else if (this->endPos == 3) {
				this->wheelPos = 2;
			}
			else if (this->endPos == 4) {
				this->wheelPos = 2.5;
			}
			else if (this->endPos == 5) {
				this->wheelPos = 3;
			}
		}
		else if (this->startPos == 2) {
			if (this->endPos == 3) {
				this->wheelPos = 2.5;
			}
			else if (this->endPos == 4) {
				this->wheelPos = 3;
			}
			else if (this->endPos == 5) {
				this->wheelPos = 3.5;
			}
		}
		else if (this->startPos == 3) {
			if (this->endPos == 4) {
				this->wheelPos = 3.5;
			}
			else if (this->endPos == 5) {
				this->wheelPos = 4;
			}
		}
		else if (this->startPos == 4) {
			if (this->endPos == 5) {
				this->wheelPos = 4.5;
			}
		}
	}
	else if (this->direction == -1) { // <-
		if (this->startPos == 5) {
			if (this->endPos == 4) {
				this->wheelPos = 4.5;
			}
			else if (this->endPos == 3) {
				this->wheelPos = 4;
			}
			else if (this->endPos == 2) {
				this->wheelPos = 3.5;
			}
			else if (this->endPos == 1) {
				this->wheelPos = 3;
			}
		}
		else if (this->startPos == 4) {
			if (this->endPos == 3) {
				this->wheelPos = 3.5;
			}
			else if (this->endPos == 2) {
				this->wheelPos = 3;
			}
			else if (this->endPos == 1) {
				this->wheelPos = 2.5;
			}
		}
		else if (this->startPos == 3) {
			if (this->endPos == 2) {
				this->wheelPos = 2.5;
			}
			else if (this->endPos == 1) {
				this->wheelPos = 2;
			}
		}
		else if (this->startPos == 2) {
			if (this->endPos == 1) {
				this->wheelPos = 1.5;
			}
		}
	}
}

/**
 * Get the position of the wheel.
 * 
 * @return the position of the wheel
 */
float WheelNote::getWheelPos() {
	return this->wheelPos;
}

/**
 * Get the number of wheel notes remaining.
 * 
 * @return number of notes remaining
 */
int WheelNote::getHoldNotesRemaining() {
	return this->notesRemaining;
}

/**
 * Get the length of the wheel note remaining.
 * 
 * @return the length remaining
 */
float WheelNote::getHoldNoteDistance() {
	return this->holdNoteDistance;
}

/**
 * Get hold notes used.
 * 
 * @return hold notes used
 */
int WheelNote::getHoldNotesUsed() {
	return this->notesUsed;
}

/**
 * Subtract one of the hold notes.
 * 
 */
void WheelNote::subtractHold() {
	this->notesRemaining -= 1;
	this->notesUsed += 1;
}

int WheelNote::getWheelSize() {
	return this->wheelSize;
}

void WheelNote::calcWheelSize() {
	this->wheelSize = abs(this->startPos - this->endPos) + 1;
}

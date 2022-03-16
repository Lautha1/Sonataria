/**
 * @file WheelNote.h
 *
 * @brief Wheel Note
 *
 * @author Julia Butenhoff
 */
#pragma once

/**
 * Holds all data for wheel style notes
 */
class WheelNote {

	private:
		bool is_slam;
		int direction;
		int startPos;
		int endPos;
		float length;
		int noteDensity;
		
		float wheelPos;
		float endTime;
		int notesRemaining;
		float holdNoteDistance;
		int notesUsed;
		int wheelSize;

	public:
		float perfectTime;
		float appearTime;
		WheelNote(float, bool, int, int, int, float, int, int);
		~WheelNote();
		bool isSlam();
		int getNoteQuantity();
		void speedChangeAppearTime(int);
		int getDirection();
		int getCrossLengthRight();
		int getCrossLengthLeft();
		float getWheelNoteLength();
		float getWheelPos();
		float calculateEndTime();
		float getEndTime();
		int getHoldNotesRemaining();
		float getHoldNoteDistance();
		int getHoldNotesUsed();
		void subtractHold();
		int getWheelSize();

		int getStartPos() const;
		int getEndPos() const;

	protected:
		void calcWheelPos();
		void calcWheelSize();
};
/**
 * @file Note.h
 *
 * @brief Note
 *
 * @author Julia Butenhoff
 */
#pragma once

/**
 * Contains all data for a regular style note
 */
class Note {

	private:
		bool is_hold;
		float holdLength;
		int noteDensity;
		float endTime;
		bool hitHold;
		int notesRemaining;
		float holdNoteDistance;
		int notesUsed;
		float yPos;

	public:
		Note(float, bool, float, int, int);
		~Note();
		bool isHold();
		int getHoldNoteQuantity();
		float perfectTime;
		float appearTime;
		void speedChangeAppearTime(int);
		void speedChangePosition();
		float calculateEndTime();
		float getEndTime();
		float getHoldLength();
		bool didHitHold();
		void setHitHold(bool);
		void subtractHold();
		float getHoldNoteDistance();
		int getHoldNotesUsed();
		int getHoldNotesRemaining();
		float getYPos();
		void setYPos(float);
};
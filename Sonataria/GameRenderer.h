/**
 * @file GameRenderer.h
 *
 * @brief Game Renderer
 *
 * @author Julia Butenhoff
 */
#pragma once
#include <vector>
using namespace std;

#include "SpriteShader.h"
#include "TextShader.h"
#include "VideoShader.h"

#include "Note.h"
#include "WheelNote.h"

class QuadSprite;
class VideoSprite;

enum JUDGEMENT {
	PERFECT_HIT,
	NEAR_HIT,
	MISS
};

/**
 * Handles all rendering when in the GAME state
 */
class GameRenderer {
	private:
		bool openGLInitialized;

		SpriteShader spriteShader;
		TextShader textShader;
		VideoShader videoShader;

		QuadSprite* track;
		QuadSprite* background;
		QuadSprite* laneNote;
		QuadSprite* wheelSlamLeft;
		QuadSprite* wheelSlamRight;

		QuadSprite* wheelPixelNote;
		QuadSprite* holdPixelNote;

		QuadSprite* noteJudgement;

		VideoSprite* testVideo;

	public:
		GameRenderer();
		~GameRenderer();
		void render(sf::RenderWindow*);

	protected:
		void drawLaneNotes(int laneNum, vector<Note>& lane, std::chrono::milliseconds currentSongOffset, int& missCount, float distance);
		void drawWheelNotes(vector<WheelNote>& wheel, std::chrono::milliseconds currentSongOffset, int& missCount, float distance);
		void drawJudgement(JUDGEMENT judgement, std::chrono::milliseconds currentSOngOffset, float& clearTime);
};
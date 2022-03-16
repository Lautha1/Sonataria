/**
 * @file GameRenderer.h
 *
 * @brief Game Renderer
 *
 * @author Julia Butenhoff
 */
#pragma once
#include <SFML/Graphics.hpp>
#include <GL/glew.h>

#include <vector>
using namespace std;

#include "QuadSprite.h"
#include "SlicedSprite.h"
#include "SpriteShader.h"
#include "TextShader.h"
#include "OpenGLFont.h"
#include "Note.h"
#include "WheelNote.h"

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

		QuadSprite* track;
		QuadSprite* background;
		QuadSprite* laneNote;
		QuadSprite* wheelSlamLeft;
		QuadSprite* wheelSlamRight;

		QuadSprite* wheelPixelNote;
		QuadSprite* holdPixelNote;

		QuadSprite* noteJudgement;

	public:
		GameRenderer();
		~GameRenderer();
		void render(sf::RenderWindow*);

	protected:
		void drawLaneNotes(int laneNum, vector<Note>& lane, std::chrono::milliseconds currentSongOffset, int& missCount, float distance);
		void drawWheelNotes(vector<WheelNote>& wheel, std::chrono::milliseconds currentSongOffset, int& missCount, float distance);
		void drawJudgement(JUDGEMENT judgement, std::chrono::milliseconds currentSOngOffset, float& clearTime);
};
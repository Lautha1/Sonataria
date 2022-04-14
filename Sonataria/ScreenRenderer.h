/**
 * @file ScreenRenderer.h
 *
 * @brief Screen Renderer
 *
 * @author Julia Butenhoff
 */
#pragma once
#include <SFML/Graphics.hpp>
#include "Song.h"
#include <vector>

#include "OpenGLFont.h"
#include "OpenGLText.h"
#include "QuadSprite.h"
#include "SpriteShader.h"
#include "TextShader.h"

/**
 * Handles all rendering except the game screen
 */
class ScreenRenderer {

	public:
		ScreenRenderer();
		~ScreenRenderer();
		void render(sf::RenderWindow*);

		void testMenuPosPlus();
		void testMenuPosMinus();
		void testMenuReset();
		int getTestMenuPos();
		void testMenuIOCheckPosPlus();
		void testMenuIOCheckPosMinus();
		int getTestMenuIOCheckPos();
		void testMenuSoundOptionsPosPlus();
		void testMenuSoundOptionsPosMinus();
		int getTestMenuSoundOptionsPos();
		void setTestMenuSoundOptionsSelected(bool newValue);
		bool getTestMenuSoundOptionsSelected();
		void testMenuNetworkingPosPlus();
		void testMenuNetworkingPosMinus();
		int getTestMenuNetworkingPos();
		void updateWheelRelation(int);
		void changeSongPage(int);
		int getSongHoverOver();
		void changeDifficultySelected(int);
		bool isCurrentSongValidToPlay();
		vector<Song> currentPageSongs;
		int getSongSelectHoverOver();
		int getDifficultyHoverOver();
		void reset();
		bool isNetworkChecking;

	private:
		int testMenuPos;
		int testMenuTotalOptions;
		int testMenuIOChkPos;
		int testMenuIOCheckTotalOptions;
		int testMenuSoundOptionsPos;
		int testMenuSoundOptionsTotalOptions;
		bool testMenuSoundOptionsSelected;
		int testMenuNetworkingPos;
		int testMenuNetworkingTotalOptions;
		int songSelectHoverOver;
		int wheelRelation;
		int numOfSongPages;
		int currentSongPage;
		vector<Song> songs;
		int difficultyHoverOver;

		bool openGLInitialized;

		SpriteShader spriteShader;
		TextShader textShader;

		QuadSprite* titleScreen;

		// General Sprites
		QuadSprite* Stage;
		QuadSprite* OpenCurtains;

		// PreLogin Sprites
		QuadSprite* LifeLinkIcon;
};

extern ScreenRenderer screenRenderer;

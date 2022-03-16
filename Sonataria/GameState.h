/**
 * @file GameState.h
 *
 * @brief Game State
 *
 * @author Julia Butenhoff
 */
#pragma once
#include "Results.h"
#include "Song.h"
#include <string>
#include <vector>
using namespace std;

/**
 * Tracks the game state the game is currently in and other data for that play session
 */
class GameState {

	public:
		enum class CurrentState {
			STARTUP, SHUTDOWN, ERROR_CODE, TEST_MENU_MAIN, TITLE_SCREEN, UPDATES, TEST_MENU_IOCHECK, 
			TEST_MENU_INPUTCHECK, TEST_MENU_SYSINFO, PRELOGIN, LOGIN_DETAILS, SONG_SELECT, GAME, RESULTS, FINAL_RESULTS, 
			TEST_MENU_SOUNDOPTIONS, TEST_MENU_NETWORKING
		};
		enum class OnlineState {
			OFFLINE, ONLINE, MAINTENENCE
		};
		GameState();
		~GameState();
		CurrentState getGameState();
		void setGameState(CurrentState);
		void setGameState(int);
		void setServicePressed(bool);
		bool checkService();
		void setSongPlaying(Song, int);
		Song getSongPlaying();
		int getSongPlayingDifficulty();
		vector<Results> results;
		void resetResults();
		void setSpeed(int);
		int getSpeed();
		void setOnlineState(OnlineState);
		OnlineState getOnlineState();
		string getOnlineStateStr();

	private:
		CurrentState state;
		OnlineState onlineState;
		int err_code;
		bool servicePressed;
		Song currentlyPlaying;
		int difficulty;
		int speed;
		void onStateUnload(CurrentState);
		void onStateLoad(CurrentState);
};

extern GameState gameState;
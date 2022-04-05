#define _USE_MATH_DEFINES
#include <cmath>

#include <fstream>
#include <chrono>
#include <iostream>
#include <thread>
using namespace std;

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>

#include <windows.h>

#include "ControllerInput.h"
#include "GameRenderer.h"
#include "GameState.h"
#include "Results.h"
#include "Logger.h"
#include "UserData.h"

#include "QuadSprite.h"
#include "VideoSprite.h"

#include "OpenGLText.h"
#include "TextureList.h"

void parseInNotes(vector<Note>& lane, int LaneNum, string songPath, int diffNumber, int bpm);
void parseInWheel(vector<WheelNote>& wheel, string songPath, int diffNumber, int bpm);
void tokenize2(std::string const& str, const char delim, std::vector<std::string>& out);
void updateAppearTime(vector<Note>& lane);
void updateAppearTime(vector<WheelNote>& lane);
void updateScreenTime(float& distance);
wstring getScoreString(float score);

std::chrono::milliseconds timespan(1000);

// Define the size of the screen
const float SCRWIDTH = 1920.f;
const float SCRHEIGHT = 1080.f;

// These only represent one side of the window
const float PERFECT_WINDOW = 45.f;
const float NEAR_WINDOW = 90.f;
const float MISS_WINDOW = 135.f;
const float SLAM_RANGE = 1.f;

// The position of each lane on screen
const float LANE_1_POS = SCRWIDTH / 2.f - 150.f;
const float LANE_2_POS = SCRWIDTH / 2.f - 75.f;
const float LANE_3_POS = SCRWIDTH / 2.f;
const float LANE_4_POS = SCRWIDTH / 2.f + 75.f;
const float LANE_5_POS = SCRWIDTH / 2.f + 150.f;

// The position between each lane
const float LANE_1_2_BETWEEN = (LANE_2_POS - LANE_1_POS) / 2.f + LANE_1_POS;
const float LANE_2_3_BETWEEN = (LANE_3_POS - LANE_2_POS) / 2.f + LANE_2_POS;
const float LANE_3_4_BETWEEN = (LANE_4_POS - LANE_3_POS) / 2.f + LANE_3_POS;
const float LANE_4_5_BETWEEN = (LANE_5_POS - LANE_4_POS) / 2.f + LANE_4_POS;

// The positions of the song scrubber
const float SCRUB_START = 3.f * SCRWIDTH / 4.f - 90.f;
const float SCRUB_END = 9.f * SCRWIDTH / 10.f + 120.f;
const float SCRUB_LENGTH = SCRUB_END - SCRUB_START;

// Where the wheel slam position is
const float WHEEL_SLAM_POS = SCRWIDTH / 2.f;

// Where the perfect line is on screen
const float DISTANCE_TO_PERFECT = 921.f;

// Where the judgement box is on screen
const float JUDGEMENT_LOC = 4.f * SCRWIDTH / 5.f;

const float laneNoteScale = 0.18f;

ifstream inputFile;

/**
 * Default constructor.
 * 
 */
GameRenderer::GameRenderer() {
	this->openGLInitialized = false;
	track = new QuadSprite(L"Track");
	background = new QuadSprite(L"Background");
	laneNote = new QuadSprite(L"Lane Note");
	wheelSlamLeft = new QuadSprite(L"Wheel Slam Left");
	wheelSlamRight = new QuadSprite(L"Wheel Slam Right");
	wheelPixelNote = new QuadSprite(L"Wheel Note in Pixel Units");
	holdPixelNote = new QuadSprite(L"Hold Note in Pixel Units");
	noteJudgement = new QuadSprite(L"Judgement for Notes");
	animatedNote = new QuadSprite(L"Texture Animation Test");
	lerpingNote = new QuadSprite(L"Testing linear interpolation");
	testVideo = new VideoSprite(L"Test video sprite");
}

/**
 * Default deconstructor.
 * 
 */
GameRenderer::~GameRenderer() {
	delete background;
	delete track;
	delete laneNote;
	delete wheelSlamLeft;
	delete wheelSlamRight;
	delete wheelPixelNote;
	delete holdPixelNote;
	delete noteJudgement;
	delete animatedNote;
	delete lerpingNote;
	delete testVideo;
}

/**
 * Handles the rendering in game.
 * 
 * @param gameWindow the game window
 */
void GameRenderer::render(sf::RenderWindow* gameWindow) {
	
	// Initialize counts
	int perfectCount = 0;
	int nearCount = 0;
	int missCount = 0;

	// Score of the song (while playing)
	float score = 0000000;

	// Used to track if there was a change in the speed
	int oldSpeed = 0;
	gameState.setSpeed(stoi(gameState.getSongPlaying().getBPM()));
	oldSpeed = gameState.getSpeed();

	// If the user has a custom speed, set it now
	if (userData.useCustomSpeed()) {
		oldSpeed = userData.getGameSpeed();
		gameState.setSpeed(oldSpeed);
	}

	// Set the judgement "clear" off screen time back to default at 0
	float clearTime = 0;

	// Use OpenGL version of graphics implementation
	logger.log(L"OpenGL Game Renderer Active.");

	// Set the game window to active in this thread
	gameWindow->setActive(true);

	// Load the Audio Files
	sf::Music countdown;
	countdown.openFromFile("MusicFX/321countdown.wav");

	sf::Music song;
	song.openFromFile(gameState.getSongPlaying().getAudioFilePath());
	float songDuration = (float)song.getDuration().asMilliseconds();

	// OpenGL Setup
	if (!this->openGLInitialized) {
		this->openGLInitialized = true;

		// Enable and configure alpha blending
		glEnable(GL_BLEND);
		glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
		glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);

		// INITIALIZE SPRITE SHADER
		if (!spriteShader.initShader()) {
			logger.logError(L"Failed to initialize sprite shader");
			exit(1);
		}

		// INITIALIZE THE SPRITES
		background->initSprite(spriteShader.getProgram());
		track->initSprite(spriteShader.getProgram());
		laneNote->initSprite(spriteShader.getProgram());
		wheelSlamLeft->initSprite(spriteShader.getProgram());
		wheelSlamRight->initSprite(spriteShader.getProgram());
		wheelPixelNote->initSprite(spriteShader.getProgram());
		holdPixelNote->initSprite(spriteShader.getProgram());
		noteJudgement->initSprite(spriteShader.getProgram());
		animatedNote->initSprite(spriteShader.getProgram());
		lerpingNote->initSprite(spriteShader.getProgram());

		// INITIALIZE THE TEXTURES
		background->setTextureID(TextureList::Inst()->GetTextureID("Textures/temp_Background.png"));
		track->setTextureID(TextureList::Inst()->GetTextureID("Textures/temp_Track.png"));
		laneNote->setTextureID(TextureList::Inst()->GetTextureID("Textures/temp_Note.png"));
		wheelSlamLeft->setTextureID(TextureList::Inst()->GetTextureID("Textures/temp_SlamLeft.png"));
		wheelSlamRight->setTextureID(TextureList::Inst()->GetTextureID("Textures/temp_SlamRight.png"));
		wheelPixelNote->setTextureID(TextureList::Inst()->GetTextureID("Textures/temp_wheel_Note.png"));
		holdPixelNote->setTextureID(TextureList::Inst()->GetTextureID("Textures/temp_hold_Note.png"));
		animatedNote->setTextureID(TextureList::Inst()->GetTextureID("Textures/MissingRoundArt.png"));
		lerpingNote->setTextureID(TextureList::Inst()->GetTextureID("Textures/MissingJacketArt.png"));

		// SET THE TRANSFORMATIONS

		// Background
		background->scale(2.f * (16.f/9.f), 2.f, 1.f);

		// Track
		track->scale(1.f, 5.f, 1.f);
		track->translate(0.f, 0.34f, -3.5f);
		track->rotate(-65.f, 0.f, 0.f);

		laneNote->scale(1.f, 0.159f, 1.f);
		laneNote->scale(laneNoteScale);
		laneNote->translate(0.f, -1.f, -3.5f);
		laneNote->rotate(-65.f, 0.f, 0.f);

		wheelSlamLeft->scale(1.f, 0.159f, 1.f);
		wheelSlamLeft->scale(laneNoteScale);
		wheelSlamLeft->translate(0.f, -1.f, -3.5f);
		wheelSlamLeft->rotate(-65.f, 0.f, 0.f);
		wheelSlamLeft->setOpacity(0.5f);

		wheelSlamRight->scale(1.f, 0.159f, 1.f);
		wheelSlamRight->scale(laneNoteScale);
		wheelSlamRight->translate(0.f, -1.f, -3.5f);
		wheelSlamRight->rotate(-65.f, 0.f, 0.f);
		wheelSlamRight->setOpacity(0.5f);

		// Judgement Sizing
		noteJudgement->scale(0.2f, .2f * 9.f / 16.f, 1.f);
		noteJudgement->translate(0.f, .5f, 0.f);
		noteJudgement->setTextureID(TextureList::Inst()->GetTextureID("Textures/Judgement/temp_Perfect.png"));
		noteJudgement->setTextureID(TextureList::Inst()->GetTextureID("Textures/Judgement/temp_Near.png"));
		noteJudgement->setTextureID(TextureList::Inst()->GetTextureID("Textures/Judgement/temp_Miss.png"));

		// Setup scale on the pixelNote sprites so they are in units of pixels
		wheelPixelNote->scale(1.0f / (.1984f * 2.f), 1.0f / 2.f, 1.0f);
		wheelPixelNote->scale(0.5773f, 1.0f, 1.0f); // sqrt(3)/3

		holdPixelNote->scale(1.0f / (.1984f * 2.f), 1.0f / 2.f, 1.0f);
		holdPixelNote->scale(0.5773f, 1.0f, 1.0f); // sqrt(3)/3

		// Test animation
		animatedNote->scale(0.33f, 0.33f, 0.33f);
		animatedNote->translate(1.33f, 0.0f, 0.0f);
		animatedNote->addMotion(PROP_POSITION, Vector3(-0.01f, 0.0f, 0.0f), 1);

		lerpingNote->scale(0.33f, 0.33f, 0.33f);
		lerpingNote->translate(-1.33f, 0.33f, 0.0f);
		lerpingNote->addInterpolation(
			INTERP_LINEAR, PROP_OPACITY,
			0.1f, 1.0f, 10000
		);
		lerpingNote->update(0);

		// INITIALIZE TEXT SHADER
		if (!textShader.initShader()) {
			logger.logError(L"Failed to initialize text shader");
			exit(1);
		}

		// INITIALIZE VIDEO SHADER
		//if (!videoShader.initShader()) {
		//	logger.logError(L"Failed to initialize video shader");
		//	exit(1);
		//}

		// Initialize videos
		//if (!testVideo->loadVideo("Songs/+ERABY+E CONNEC+10N/+ERABY+ECONNEC+10N.mp4"))
		//{
		//	logger.logError("Failed to load video 'Songs/+ERABY+E CONNEC+10N/+ERABY+ECONNEC+10N.mp4'");
		//}

		// Link video sprite and shader
		//testVideo->initSprite(videoShader.getProgram());

		// Load first frame
		//testVideo->scale(2.f * (16.f / 9.f), -2.f, 1.f);
		//testVideo->update(0);
	}

	// INITIALIZE TEXT
	TextureManager::TextureInfo fontInfo = TextureList::Inst()->GetTextureInfo("Fonts/HonyaJi-Re.ttf");

	OpenGLText* songTitle = new OpenGLText(L"Song Title", *fontInfo.font);
	songTitle->initSprite(textShader.getProgram());
	songTitle->translate(-3300.f, 950.f, 0.f);

	OpenGLText* scoreText = new OpenGLText(L"Score", *fontInfo.font);
	scoreText->initSprite(textShader.getProgram());
	scoreText->translate(2600.f, 950.f, 0.f);

	OpenGLText* speedText = new OpenGLText(L"Speed", *fontInfo.font);
	speedText->initSprite(textShader.getProgram());
	speedText->translate(-3300.f, 800.f, 0.f);
	speedText->scale(0.75f);

	// Clear Color for Background
	glClearColor(0.968627f, 0.658823f, 0.721568f, 1.0f);
	glViewport(0, 0, 1920, 1080);

	vector<Note> lane1;
	vector<Note> lane2;
	vector<Note> lane3;
	vector<Note> lane4;
	vector<Note> lane5;
	vector<WheelNote> wheel;

	logger.log(L"Reading in notes...");

	parseInNotes(lane1, 1, gameState.getSongPlaying().getPath(), gameState.getSongPlayingDifficulty(), gameState.getSpeed());
	parseInNotes(lane2, 2, gameState.getSongPlaying().getPath(), gameState.getSongPlayingDifficulty(), gameState.getSpeed());
	parseInNotes(lane3, 3, gameState.getSongPlaying().getPath(), gameState.getSongPlayingDifficulty(), gameState.getSpeed());
	parseInNotes(lane4, 4, gameState.getSongPlaying().getPath(), gameState.getSongPlayingDifficulty(), gameState.getSpeed());
	parseInNotes(lane5, 5, gameState.getSongPlaying().getPath(), gameState.getSongPlayingDifficulty(), gameState.getSpeed());
	parseInWheel(wheel, gameState.getSongPlaying().getPath(), gameState.getSongPlayingDifficulty(), gameState.getSpeed());

	// Calculate the total notes by adding each lane together
	int totalNotes = 0;
	{
		for (size_t i = 0; i < lane1.size(); i++) {
			if (lane1[i].isHold()) {
				totalNotes += lane1[i].getHoldNoteQuantity();
			}
			else {
				totalNotes += 1;
			}
		}

		for (size_t i = 0; i < lane2.size(); i++) {
			if (lane2[i].isHold()) {
				totalNotes += lane2[i].getHoldNoteQuantity();
			}
			else {
				totalNotes += 1;
			}
		}

		for (size_t i = 0; i < lane3.size(); i++) {
			if (lane3[i].isHold()) {
				totalNotes += lane3[i].getHoldNoteQuantity();
			}
			else {
				totalNotes += 1;
			}
		}

		for (size_t i = 0; i < lane4.size(); i++) {
			if (lane4[i].isHold()) {
				totalNotes += lane4[i].getHoldNoteQuantity();
			}
			else {
				totalNotes += 1;
			}
		}

		for (size_t i = 0; i < lane5.size(); i++) {
			if (lane5[i].isHold()) {
				totalNotes += lane5[i].getHoldNoteQuantity();
			}
			else {
				totalNotes += 1;
			}
		}

		for (size_t i = 0; i < wheel.size(); i++) {
			if (wheel[i].isSlam()) {
				totalNotes += 1;
			}
			else {
				totalNotes += wheel[i].getNoteQuantity();
			}
		}
	}
	logger.log(L"Total Notes: " + to_wstring(totalNotes));

	// Calculate the value that each note is worth based on the total notes
	float perfectNoteScore = 1000000.f / (float)totalNotes;
	float nearNoteScore = perfectNoteScore / 2.f;

	float distance = (float)(453075.9705 / pow(oldSpeed, 1.004140998));

	int itemFront = NULL;

	// Clear the input queue before the song starts
	controllerInput.queue_mutexLock.lock();
	controllerInput.inputQueue.clear();
	controllerInput.queue_mutexLock.unlock();

	// Clear the wheel last before start
	controllerInput.resetLast();

	// Play the countdown
	countdown.play();

	// Do a render loop while playing to display the 3.2.1. countdown on screen
	while (countdown.getStatus() == sf::Music::Status::Playing) {

		// If the service button was pressed, stop the song
		if (gameState.checkService() || gameState.getGameState() == GameState::CurrentState::SHUTDOWN) {
			logger.log(L"Ending Game Render Loop.");
			countdown.stop();
			continue;
		}
		else {
			// Draw the sprites
			glUseProgram(spriteShader.getProgram());

			// Draw the background
			background->render(PROJECTION::ORTHOGRAPHIC);

			// Clear the depth buffer
			glClear(GL_DEPTH_BUFFER_BIT);

			// Test animation sprite
			animatedNote->render(PROJECTION::ORTHOGRAPHIC);
			lerpingNote->render(PROJECTION::ORTHOGRAPHIC);

			// Draw track and track sprites
			track->render(PROJECTION::PERSPECTIVE);

			// Draw all text
			glUseProgram(textShader.getProgram());
			songTitle->render(PROJECTION::ORTHOGRAPHIC, gameState.getSongPlaying().getTitle(), ALIGNMENT::LEFT, 1.f, 1.f, 1.f, 1.5f);
			scoreText->render(PROJECTION::ORTHOGRAPHIC, getScoreString(score), ALIGNMENT::LEFT, 1.f, 1.f, 1.f, 1.5f);
			if (controllerInput.getKeyboardState().getKeyState(6)) {
				speedText->render(PROJECTION::ORTHOGRAPHIC, to_wstring(gameState.getSpeed()), ALIGNMENT::LEFT, 0.f, 1.f, 0.f, 1.5f);
			}
			else {
				speedText->render(PROJECTION::ORTHOGRAPHIC, to_wstring(gameState.getSpeed()), ALIGNMENT::LEFT, 1.f, 1.f, 1.f, 1.5f);
			}

			gameWindow->display();
		}
	}

	// After countdown play the song
	song.play();

	// Create time variables
	typedef std::chrono::high_resolution_clock Time;
	typedef std::chrono::milliseconds ms;
	typedef std::chrono::duration<float> fsec;
	auto songStartTime = Time::now();

	// Do a render loop while playing the song
	while (song.getStatus() == sf::Music::Status::Playing) {
		
		// Set the current position in the song
		auto currentSongPosition = Time::now();
		fsec fs = currentSongPosition - songStartTime;
		ms currentSongOffset = std::chrono::duration_cast<ms>(fs);
		// Use currentSongOffset.count() to get millisecond value

		// Allow video to update
		//testVideo->update((double)fs.count());

		// If the service button was pressed, stop the song
		if (gameState.checkService() || gameState.getGameState() == GameState::CurrentState::SHUTDOWN) {
			logger.log(L"Ending Game Render Loop.");
			song.stop();
			continue;
		}
		else {
			// ** INPUT **
			glUseProgram(spriteShader.getProgram());

			// Check for new inputs
			if (!controllerInput.inputQueue.empty()) {
				// Lock the mutex
				controllerInput.queue_mutexLock.lock();

				// Store the front item in the queue
				itemFront = controllerInput.inputQueue.front();
				
				// Remove that item from the queue
				controllerInput.inputQueue.pop_front();

				// Unlock the mutex
				controllerInput.queue_mutexLock.unlock();
			}

			// Handle an input that was pulled off the queue
			if (itemFront != NULL) {
				switch (itemFront) {
					// Logic for Lane 1
					case 1:
						// Note (not hold)
						if (!lane1.empty() && !lane1[0].isHold() && lane1[0].getYPos() + MISS_WINDOW >= DISTANCE_TO_PERFECT) {
							float dist = lane1[0].perfectTime - currentSongOffset.count();
							
							if (dist <= PERFECT_WINDOW && dist >= -PERFECT_WINDOW) {
								// Perfect hit window
								score += perfectNoteScore;
								perfectCount++;

								drawJudgement(JUDGEMENT::PERFECT_HIT, currentSongOffset, clearTime);
								
								// Remove note
								lane1.erase(lane1.begin());
							}
							else if (dist <= NEAR_WINDOW && dist >= -NEAR_WINDOW) {
								// Near hit window
								score += nearNoteScore;
								nearCount++;

								drawJudgement(JUDGEMENT::NEAR_HIT, currentSongOffset, clearTime);
								
								// Remove note
								lane1.erase(lane1.begin());
							}
							else if (dist <= MISS_WINDOW) {
								// Miss window
								missCount++;

								drawJudgement(JUDGEMENT::MISS, currentSongOffset, clearTime);
								
								// Remove note
								lane1.erase(lane1.begin());
							}
						} 
						else if (!lane1.empty() && lane1[0].isHold()) { // Hold Note
							float dist = lane1[0].perfectTime - currentSongOffset.count();

							if (dist >= 0) { // Hold hasn't started yet
								if (dist <= NEAR_WINDOW) { // Inside window to start hold
									score += perfectNoteScore;
									perfectCount++;

									drawJudgement(JUDGEMENT::PERFECT_HIT, currentSongOffset, clearTime);

									lane1[0].subtractHold();
									lane1[0].setHitHold(true);
								}
								else if (dist <= MISS_WINDOW) { // Miss window to start hold
									missCount++;

									drawJudgement(JUDGEMENT::MISS, currentSongOffset, clearTime);

									lane1[0].subtractHold();
									lane1[0].setHitHold(false);
								}
							}
						}
						break;
					// Logic for Lane 2
					case 2:
						// Note (not hold)
						if (!lane2.empty() && !lane2[0].isHold() && lane2[0].getYPos() + MISS_WINDOW >= DISTANCE_TO_PERFECT) {
							float dist = lane2[0].perfectTime - currentSongOffset.count();

							if (dist <= PERFECT_WINDOW && dist >= -PERFECT_WINDOW) {
								// Perfect hit window
								score += perfectNoteScore;
								perfectCount++;

								drawJudgement(JUDGEMENT::PERFECT_HIT, currentSongOffset, clearTime);

								// Remove note
								lane2.erase(lane2.begin());
							}
							else if (dist <= NEAR_WINDOW && dist >= -NEAR_WINDOW) {
								// Near hit window
								score += nearNoteScore;
								nearCount++;

								drawJudgement(JUDGEMENT::NEAR_HIT, currentSongOffset, clearTime);

								// Remove note
								lane2.erase(lane2.begin());
							}
							else if (dist <= MISS_WINDOW) {
								// Miss window
								missCount++;

								drawJudgement(JUDGEMENT::MISS, currentSongOffset, clearTime);

								// Remove note
								lane2.erase(lane2.begin());
							}
						}
						else if (!lane2.empty() && lane2[0].isHold()) { // Hold Note
							float dist = lane2[0].perfectTime - currentSongOffset.count();

							if (dist >= 0) { // Hold hasn't started yet
								if (dist <= NEAR_WINDOW) { // Inside window to start hold
									score += perfectNoteScore;
									perfectCount++;

									drawJudgement(JUDGEMENT::PERFECT_HIT, currentSongOffset, clearTime);

									lane2[0].subtractHold();
									lane2[0].setHitHold(true);
								}
								else if (dist <= MISS_WINDOW) { // Miss window to start hold
									missCount++;

									drawJudgement(JUDGEMENT::MISS, currentSongOffset, clearTime);

									lane2[0].subtractHold();
									lane2[0].setHitHold(false);
								}
							}
						}
						break;
					// Logic for Lane 3
					case 3:
						// Note (not hold)
						if (!lane3.empty() && !lane3[0].isHold() && lane3[0].getYPos() + MISS_WINDOW >= DISTANCE_TO_PERFECT) {
							float dist = lane3[0].perfectTime - currentSongOffset.count();

							if (dist <= PERFECT_WINDOW && dist >= -PERFECT_WINDOW) {
								// Perfect hit window
								score += perfectNoteScore;
								perfectCount++;

								drawJudgement(JUDGEMENT::PERFECT_HIT, currentSongOffset, clearTime);

								// Remove Note
								lane3.erase(lane3.begin());
							}
							else if (dist <= NEAR_WINDOW && dist >= -NEAR_WINDOW) {
								// Near hit window
								score += nearNoteScore;
								nearCount++;

								drawJudgement(JUDGEMENT::NEAR_HIT, currentSongOffset, clearTime);

								// Remove Note
								lane3.erase(lane3.begin());
							}
							else if (dist <= MISS_WINDOW) {
								// Miss window
								missCount++;

								drawJudgement(JUDGEMENT::MISS, currentSongOffset, clearTime);

								// Remove Note
								lane3.erase(lane3.begin());
							}
						}
						else if (!lane3.empty() && lane3[0].isHold()) { // Hold Note
							float dist = lane3[0].perfectTime - currentSongOffset.count();

							if (dist >= 0) { // Hold hasn't started yet
								if (dist <= NEAR_WINDOW) { // Inside window to start hold
									score += perfectNoteScore;
									perfectCount++;

									drawJudgement(JUDGEMENT::PERFECT_HIT, currentSongOffset, clearTime);

									lane3[0].subtractHold();
									lane3[0].setHitHold(true);
								}
								else if (dist <= MISS_WINDOW) { // Miss window to start hold
									missCount++;

									drawJudgement(JUDGEMENT::MISS, currentSongOffset, clearTime);

									lane3[0].subtractHold();
									lane3[0].setHitHold(false);
								}
							}
						}
						break;
					// Logic for Lane 4
					case 4:
						// Note (not hold)
						if (!lane4.empty() && !lane4[0].isHold() && lane4[0].getYPos() + MISS_WINDOW >= DISTANCE_TO_PERFECT) {
							float dist = lane4[0].perfectTime - currentSongOffset.count();

							if (dist <= PERFECT_WINDOW && dist >= -PERFECT_WINDOW) {
								// Perfect hit window
								score += perfectNoteScore;
								perfectCount++;

								drawJudgement(JUDGEMENT::PERFECT_HIT, currentSongOffset, clearTime);

								// Remove note
								lane4.erase(lane4.begin());
							}
							else if (dist <= NEAR_WINDOW && dist >= -NEAR_WINDOW) {
								// Near hit window
								score += nearNoteScore;
								nearCount++;

								drawJudgement(JUDGEMENT::NEAR_HIT, currentSongOffset, clearTime);

								// Remove note
								lane4.erase(lane4.begin());
							} 
							else if (dist <= MISS_WINDOW) {
								// Miss window
								missCount++;

								drawJudgement(JUDGEMENT::MISS, currentSongOffset, clearTime);

								// Remove note
								lane4.erase(lane4.begin());
							}
						}
						else if (!lane4.empty() && lane4[0].isHold()) { // Hold Note
							float dist = lane4[0].perfectTime - currentSongOffset.count();

							if (dist >= 0) { // Hold hasn't started yet
								if (dist <= NEAR_WINDOW) { // Inside window to start hold
									score += perfectNoteScore;
									perfectCount++;

									drawJudgement(JUDGEMENT::PERFECT_HIT, currentSongOffset, clearTime);

									lane4[0].subtractHold();
									lane4[0].setHitHold(true);
								}
								else if (dist <= MISS_WINDOW) { // Miss window to start hold
									missCount++;

									drawJudgement(JUDGEMENT::MISS, currentSongOffset, clearTime);

									lane4[0].subtractHold();
									lane4[0].setHitHold(false);
								}
							}
						}
						break;
					// Logic for Lane 5
					case 5:
						// Note (not hold)
						if (!lane5.empty() && !lane5[0].isHold() && lane5[0].getYPos() + MISS_WINDOW >= DISTANCE_TO_PERFECT) {
							float dist = lane5[0].perfectTime - currentSongOffset.count();

							if (dist <= PERFECT_WINDOW && dist >= -PERFECT_WINDOW) {
								// Perfect hit window
								score += perfectNoteScore;
								perfectCount++;

								drawJudgement(JUDGEMENT::PERFECT_HIT, currentSongOffset, clearTime);

								// Remove note
								lane5.erase(lane5.begin());
							}
							else if (dist <= NEAR_WINDOW && dist >= -NEAR_WINDOW) {
								// Near hit window
								score += nearNoteScore;
								nearCount++;

								drawJudgement(JUDGEMENT::NEAR_HIT, currentSongOffset, clearTime);

								// Remove note
								lane5.erase(lane5.begin());
							}
							else if (dist <= MISS_WINDOW) {
								// Miss window
								missCount++;

								drawJudgement(JUDGEMENT::MISS, currentSongOffset, clearTime);

								// Remove note
								lane5.erase(lane5.begin());
							}
						}
						else if (!lane5.empty() && lane5[0].isHold()) { // Hold Note
							float dist = lane5[0].perfectTime - currentSongOffset.count();

							if (dist >= 0) { // Hold hasn't started yet
								if (dist <= NEAR_WINDOW) { // Inside window to start hold
									score += perfectNoteScore++;
									perfectCount++;

									drawJudgement(JUDGEMENT::PERFECT_HIT, currentSongOffset, clearTime);

									lane5[0].subtractHold();
									lane5[0].setHitHold(true);
								}
								else if (dist <= MISS_WINDOW) { // Miss window to start hold
									missCount++;

									drawJudgement(JUDGEMENT::MISS, currentSongOffset, clearTime);

									lane5[0].subtractHold();
									lane5[0].setHitHold(false);
								}
							}
						}
						break;
				}

				// After handling hit, set things back to null
				itemFront = NULL;
			}
			
			// Handle the remaining parts of the hold notes next

			// Lane 1 Hold
			if (!lane1.empty() && lane1[0].isHold()) {
				float dist = lane1[0].perfectTime - currentSongOffset.count();
				if (!(lane1[0].getHoldNotesRemaining() <= 0)) { // Check if the hold has any notes remaining
					// Now that we know there are notes remaining
					// Check if holding or dropped

					float nextNote = lane1[0].perfectTime + (lane1[0].getHoldNoteDistance() * (float)lane1[0].getHoldNotesUsed());
					float pastDist = nextNote - currentSongOffset.count();

					if (dist < 0 && lane1[0].didHitHold()) { // Hold has started, and holding
						if (pastDist <= 0) {
							score += perfectNoteScore;
							perfectCount++;

							drawJudgement(JUDGEMENT::PERFECT_HIT, currentSongOffset, clearTime);

							lane1[0].subtractHold();
						}
					}
					else if (dist < 0 && !lane1[0].didHitHold()) { // Hold has started, and not holding
						if (pastDist <= 0) {
							missCount++;

							drawJudgement(JUDGEMENT::MISS, currentSongOffset, clearTime);

							lane1[0].subtractHold();
						}
					}

					// Update hold status
					if (currentSongOffset.count() > lane1[0].perfectTime && currentSongOffset.count() <= lane1[0].getEndTime()) {
						if (controllerInput.getKeyboardState().getKeyState(1)) { // Button 1 Pressed
							if (!lane1[0].didHitHold()) {
								lane1[0].setHitHold(true);
							}
						}
						else { // Button 1 Released
							if (lane1[0].didHitHold()) {
								lane1[0].setHitHold(false);
							}
						}
					}
				}
			}

			// Lane 2 Hold
			if (!lane2.empty() && lane2[0].isHold()) {
				float dist = lane2[0].perfectTime - currentSongOffset.count();
				if (!(lane2[0].getHoldNotesRemaining() <= 0)) { // Check if the hold has any notes remaining
					// Notw that we know there are notes remaining
					// Check if holding or dropped

					float nextNote = lane2[0].perfectTime + (lane2[0].getHoldNoteDistance() * (float)lane2[0].getHoldNotesUsed());
					float pastDist = nextNote - currentSongOffset.count();

					if (dist < 0 && lane2[0].didHitHold()) { // Hold has started, and holding
						if (pastDist <= 0) {
							score += perfectNoteScore;
							perfectCount++;

							drawJudgement(JUDGEMENT::PERFECT_HIT, currentSongOffset, clearTime);

							lane2[0].subtractHold();
						}
					}
					else if (dist < 0 && !lane2[0].didHitHold()) { // Hold has started, and not holding
						if (pastDist <= 0) {
							missCount++;

							drawJudgement(JUDGEMENT::MISS, currentSongOffset, clearTime);

							lane2[0].subtractHold();
						}
					}

					// Update hold status
					if (currentSongOffset.count() > lane2[0].perfectTime && currentSongOffset.count() <= lane2[0].getEndTime()) {
						if (controllerInput.getKeyboardState().getKeyState(2)) { // Button 2 Pressed
							if (!lane2[0].didHitHold()) {
								lane2[0].setHitHold(true);
							}
						}
						else { // Button 2 Released
							if (lane2[0].didHitHold()) {
								lane2[0].setHitHold(false);
							}
						}
					}
				}
			}

			// Lane 3 Hold
			if (!lane3.empty() && lane3[0].isHold()) {
				float dist = lane3[0].perfectTime - currentSongOffset.count();
				if (!(lane3[0].getHoldNotesRemaining() <= 0)) { // Check if the hold has any notes remaining
					// Now that we know there are notes remaining
					// Check if holding or dropped

					float nextNote = lane3[0].perfectTime + (lane3[0].getHoldNoteDistance() * (float)lane3[0].getHoldNotesUsed());
					float pastDist = nextNote - currentSongOffset.count();

					if (dist < 0 && lane3[0].didHitHold()) { // Hold has started, and holding
						if (pastDist <= 0) {
							score += perfectNoteScore;
							perfectCount++;

							drawJudgement(JUDGEMENT::PERFECT_HIT, currentSongOffset, clearTime);

							lane3[0].subtractHold();
						}
					}
					else if (dist < 0 && !lane3[0].didHitHold()) { // Hold has started, and not holding
						if (pastDist <= 0) {
							missCount++;

							drawJudgement(JUDGEMENT::MISS, currentSongOffset, clearTime);

							lane3[0].subtractHold();
						}
					}

					// Update hold status
					if (currentSongOffset.count() > lane3[0].perfectTime && currentSongOffset.count() <= lane3[0].getEndTime()) {
						if (controllerInput.getKeyboardState().getKeyState(3)) { // Button 3 Pressed
							if (!lane3[0].didHitHold()) {
								lane3[0].setHitHold(true);
							}
						}
						else {
							if (lane3[0].didHitHold()) {
								lane3[0].setHitHold(false);
							}
						}
					}
				}
			}

			// Lane 4 Hold
			if (!lane4.empty() && lane4[0].isHold()) {
				float dist = lane4[0].perfectTime - currentSongOffset.count();
				if (!(lane4[0].getHoldNotesRemaining() <= 0)) { // Check if the hold has any notes remaining
					// Now that we know there are notes remaining
					// Check if holding or dropped

					float nextNote = lane4[0].perfectTime + (lane4[0].getHoldNoteDistance() * (float)lane4[0].getHoldNotesUsed());
					float pastDist = nextNote - currentSongOffset.count();

					if (dist < 0 && lane4[0].didHitHold()) { // Hold has started, and holding
						if (pastDist <= 0) {
							score += perfectNoteScore;
							perfectCount++;

							drawJudgement(JUDGEMENT::PERFECT_HIT, currentSongOffset, clearTime);

							lane4[0].subtractHold();
						}
					}
					else if (dist < 0 && !lane4[0].didHitHold()) { // Hold has started, and not holding
						if (pastDist <= 0) {
							missCount++;

							drawJudgement(JUDGEMENT::MISS, currentSongOffset, clearTime);

							lane4[0].subtractHold();
						}
					}

					// Update hold status
					if (currentSongOffset.count() > lane4[0].perfectTime && currentSongOffset.count() <= lane4[0].getEndTime()) {
						if (controllerInput.getKeyboardState().getKeyState(4)) { // Button 4 Pressed
							if (!lane4[0].didHitHold()) {
								lane4[0].setHitHold(true);
							}
						}
						else { // Button 4 Released
							if (lane4[0].didHitHold()) {
								lane4[0].setHitHold(false);
							}
						}
					}
				}
			}

			// Lane 5 Hold
			if (!lane5.empty() && lane5[0].isHold()) {
				float dist = lane5[0].perfectTime - currentSongOffset.count();
				if (!(lane5[0].getHoldNotesRemaining() <= 0)) { // Check if the hold has any notes remaining
					// Now that we know there are notes remaining
					// Check if holding or stopped

					float nextNote = lane5[0].perfectTime + (lane5[0].getHoldNoteDistance() * (float)lane5[0].getHoldNotesUsed());
					float pastDist = nextNote - currentSongOffset.count();

					if (dist < 0 && lane5[0].didHitHold()) { // Hold has started, and holding
						if (pastDist <= 0) {
							score += perfectNoteScore;
							perfectCount++;

							drawJudgement(JUDGEMENT::PERFECT_HIT, currentSongOffset, clearTime);

							lane5[0].subtractHold();
						}
					}
					else if (dist < 0 && !lane5[0].didHitHold()) { // Hold has started, and not holding
						if (pastDist <= 0) {
							missCount++;

							drawJudgement(JUDGEMENT::MISS, currentSongOffset, clearTime);

							lane5[0].subtractHold();
						}
					}

					// Update hold status
					if (currentSongOffset.count() > lane5[0].perfectTime && currentSongOffset.count() <= lane5[0].getEndTime()) {
						if (controllerInput.getKeyboardState().getKeyState(5)) { // Button 5 Pressed
							if (!lane5[0].didHitHold()) {
								lane5[0].setHitHold(true);
							}
						}
						else { // Button 5 Released
							if (lane5[0].didHitHold()) {
								lane5[0].setHitHold(false);
							}
						}
					}
				}
			}

			// Wheel Input
			if (!wheel.empty()) {
				float dist = wheel[0].perfectTime - currentSongOffset.count();
				if (wheel[0].isSlam()) { // Slam Notes
					// If note is within range
					if (dist <= SLAM_RANGE && dist >= -SLAM_RANGE) {
						switch (wheel[0].getDirection()) {
							case 1: // Slam -> Direction
								if (controllerInput.getWheelMovement() == 1) {
									score += perfectNoteScore;
									perfectCount++;

									drawJudgement(JUDGEMENT::PERFECT_HIT, currentSongOffset, clearTime);

									// Remove Note
									wheel.erase(wheel.begin());
								}
								else {
									missCount++;

									drawJudgement(JUDGEMENT::MISS, currentSongOffset, clearTime);

									// Remove Note
									wheel.erase(wheel.begin());
								}
								
								// If there are no more wheel notes on screen, reset the last wheel position
								if (wheel.empty()) {
									controllerInput.resetLast();
								}
								break;
							case -1: // Slame <- Direction
								if (controllerInput.getWheelMovement() == -1) {
									score += perfectNoteScore;
									perfectCount++;

									drawJudgement(JUDGEMENT::PERFECT_HIT, currentSongOffset, clearTime);

									// Remove Note
									wheel.erase(wheel.begin());
								}
								else {
									missCount++;

									drawJudgement(JUDGEMENT::MISS, currentSongOffset, clearTime);

									// Remove Note
									wheel.erase(wheel.begin());
								}

								// If there are no more wheel notes on screen, reset the last wheel position
								if (wheel.empty()) {
									controllerInput.resetLast();
								}
								break;
						}
					}
				} 
				else { // Continuous Notes
					if (!(wheel[0].getHoldNotesRemaining() <= 0)) {
						if (dist < 0) {
							float nextNote = wheel[0].perfectTime + (wheel[0].getHoldNoteDistance() * (float)wheel[0].getHoldNotesUsed());
							float pastDist = nextNote - currentSongOffset.count();

							if (pastDist <= 0) {
								if (wheel[0].getDirection() == controllerInput.getWheelMovement()) {
									score += perfectNoteScore;
									perfectCount++;

									drawJudgement(JUDGEMENT::PERFECT_HIT, currentSongOffset, clearTime);

									wheel[0].subtractHold();
								}
								else {
									missCount++;

									drawJudgement(JUDGEMENT::MISS, currentSongOffset, clearTime);

									wheel[0].subtractHold();
								}

								// Reset the last wheel position before the next note check
								controllerInput.resetLast();
							}
						}
					}
				}
			}

			// Delete notes if too far
			{
				// Lane 1 Note
				while (!lane1.empty() && !lane1[0].isHold() && lane1[0].getYPos() > DISTANCE_TO_PERFECT + MISS_WINDOW && currentSongOffset.count() > lane1[0].getEndTime() + MISS_WINDOW) {
					missCount++;

					drawJudgement(JUDGEMENT::MISS, currentSongOffset, clearTime);

					lane1.erase(lane1.begin());
				}

				// Lane 1 Hold
				while (!lane1.empty() && lane1[0].isHold() && lane1[0].getYPos() > DISTANCE_TO_PERFECT + MISS_WINDOW && currentSongOffset.count() > lane1[0].getEndTime() + MISS_WINDOW) {
					// Don't need to apply a miss as the holds miss are taken care of elsewhere

					lane1.erase(lane1.begin());
				}

				// Lane 2 Note
				while (!lane2.empty() && !lane2[0].isHold() && lane2[0].getYPos() > DISTANCE_TO_PERFECT + MISS_WINDOW && currentSongOffset.count() > lane2[0].getEndTime() + MISS_WINDOW) {
					missCount++;

					drawJudgement(JUDGEMENT::MISS, currentSongOffset, clearTime);

					lane2.erase(lane2.begin());
				}

				// Lane 2 Hold
				while (!lane2.empty() && lane2[0].isHold() && lane2[0].getYPos() > DISTANCE_TO_PERFECT + MISS_WINDOW && currentSongOffset.count() > lane2[0].getEndTime() + MISS_WINDOW) {
					// Don't need to apply a miss as the holds miss are taken care of elsewhere

					lane2.erase(lane2.begin());
				}

				// Lane 3 Note
				while (!lane3.empty() && !lane3[0].isHold() && lane3[0].getYPos() > DISTANCE_TO_PERFECT + MISS_WINDOW && currentSongOffset.count() > lane3[0].getEndTime() + MISS_WINDOW) {
					missCount++;

					drawJudgement(JUDGEMENT::MISS, currentSongOffset, clearTime);

					lane3.erase(lane3.begin());
				}

				// Lane 3 Hold
				while (!lane3.empty() && lane3[0].isHold() && lane3[0].getYPos() > DISTANCE_TO_PERFECT + MISS_WINDOW && currentSongOffset.count() > lane3[0].getEndTime() + MISS_WINDOW) {
					// Don't need to apply a miss as the holds miss are taken care of elsewhere

					lane3.erase(lane3.begin());
				}

				// Lane 4 Note
				while (!lane4.empty() && !lane4[0].isHold() && lane4[0].getYPos() > DISTANCE_TO_PERFECT + MISS_WINDOW && currentSongOffset.count() > lane4[0].getEndTime() + MISS_WINDOW) {
					missCount++;

					drawJudgement(JUDGEMENT::MISS, currentSongOffset, clearTime);

					lane4.erase(lane4.begin());
				}

				// Lane 4 Hold
				while (!lane4.empty() && lane4[0].isHold() && lane4[0].getYPos() > DISTANCE_TO_PERFECT + MISS_WINDOW && currentSongOffset.count() > lane4[0].getEndTime() + MISS_WINDOW) {
					// Don't need to apply a miss as the holds miss are taken care of elsewhere

					lane4.erase(lane4.begin());
				}

				// Lane 5 Note
				while (!lane5.empty() && !lane5[0].isHold() && lane5[0].getYPos() > DISTANCE_TO_PERFECT + MISS_WINDOW && currentSongOffset.count() > lane5[0].getEndTime() + MISS_WINDOW) {
					missCount++;

					drawJudgement(JUDGEMENT::MISS, currentSongOffset, clearTime);

					lane5.erase(lane5.begin());
				}

				// Lane 5 Hold
				while (!lane5.empty() && lane5[0].isHold() && lane5[0].getYPos() > DISTANCE_TO_PERFECT + MISS_WINDOW && currentSongOffset.count() > lane5[0].getEndTime() + MISS_WINDOW) {
					// Don't need to apply a miss as the holds miss are taken care of elsewhere

					lane5.erase(lane5.begin());
				}

				// Wheel Continuous
				while (!wheel.empty() && !wheel[0].isSlam() && currentSongOffset.count() > wheel[0].getEndTime()) {
					// Don't need to apply a miss as the miss is taken care of elsewhere

					wheel.erase(wheel.begin());

					// Reset controller last
					controllerInput.resetLast();
				}

				// Wheel slams are taken care of in the input section
			}
				
			// ** END INPUT **

			// Render Background
			glUseProgram(spriteShader.getProgram());
			background->render(PROJECTION::ORTHOGRAPHIC);

			// Draw background video
			//glUseProgram(videoShader.getProgram());
			//testVideo->render(PROJECTION::ORTHOGRAPHIC);

			// ** MAKE SURE TO MIND ORDER IF YOU REDRAW ANYTHING **

			// Clear the depth buffer
			glClear(GL_DEPTH_BUFFER_BIT);

			// Use the sprite shader
			glUseProgram(spriteShader.getProgram());

			// Test animation sprite
			animatedNote->update(currentSongOffset.count());
			animatedNote->render(PROJECTION::ORTHOGRAPHIC);

			lerpingNote->update(currentSongOffset.count());
			lerpingNote->render(PROJECTION::ORTHOGRAPHIC);

			// Draw judgement text
			if (currentSongOffset.count() < clearTime) {
				noteJudgement->render(PROJECTION::ORTHOGRAPHIC);
			}

			// Draw all other graphics
			track->render(PROJECTION::PERSPECTIVE);
			track->pushModelMatrix();
			drawLaneNotes(1, lane1, currentSongOffset, missCount, distance);
			drawLaneNotes(2, lane2, currentSongOffset, missCount, distance);
			drawLaneNotes(3, lane3, currentSongOffset, missCount, distance);
			drawLaneNotes(4, lane4, currentSongOffset, missCount, distance);
			drawLaneNotes(5, lane5, currentSongOffset, missCount, distance);
			drawWheelNotes(wheel, currentSongOffset, missCount, distance);
			OpenGLSprite::popMatrix();

			// Draw all text
			glUseProgram(textShader.getProgram());
			songTitle->render(PROJECTION::ORTHOGRAPHIC, gameState.getSongPlaying().getTitle(), ALIGNMENT::LEFT, 1.f, 1.f, 1.f, 1.5f);
			scoreText->render(PROJECTION::ORTHOGRAPHIC, getScoreString(score), ALIGNMENT::LEFT, 1.f, 1.f, 1.f, 1.5f);
			if (controllerInput.getKeyboardState().getKeyState(6)) {
				speedText->render(PROJECTION::ORTHOGRAPHIC, to_wstring(gameState.getSpeed()), ALIGNMENT::LEFT, 0.f, 1.f, 0.f, 1.5f);
			}
			else {
				speedText->render(PROJECTION::ORTHOGRAPHIC, to_wstring(gameState.getSpeed()), ALIGNMENT::LEFT, 1.f, 1.f, 1.f, 1.5f);
			}

			gameWindow->display();

			// Handle speed change
			if (oldSpeed != gameState.getSpeed()) {
				// Update Distance
				updateScreenTime(distance);

				// Update the appear times for each lane
				updateAppearTime(lane1);
				updateAppearTime(lane2);
				updateAppearTime(lane3);
				updateAppearTime(lane4);
				updateAppearTime(lane5);
				updateAppearTime(wheel);

				oldSpeed = gameState.getSpeed();
			}
		}
	}
	// End of Song

	// Only save the status of the song when the service button wasn't pressed
	if (!gameState.checkService() && gameState.getGameState() != GameState::CurrentState::SHUTDOWN) {
		// Sleep the thread for 2 seconds after the song ends
		this_thread::sleep_for(2 * timespan);

		logger.log(L"Song Ended - Game Renderer Shutting Down.");

		// Make a new results object based on how the player did on that song
		Results songResult(gameState.getSongPlaying(), gameState.getSongPlayingDifficulty(), (int)score, perfectCount, nearCount, missCount);

		// Store the results in the game state
		gameState.results.push_back(songResult);

		logger.log(L"Results of song saved.");
	}

	// Need to clear the VAO to clean it up after drawing
	glBindVertexArray(0);
	gameWindow->pushGLStates();

	// Set the game window to false
	gameWindow->setActive(false);

	// Return back to the screen renderer
}

/**
 * Read in notes from a file.
 * 
 * @param lane Lane vector to store the notes in
 * @param LaneNum Lane number to get the notes about
 * @param songPath Path of the song
 * @param diffNumber Difficulty of the song
 * @param bpm BPM of the song
 */
void parseInNotes(vector<Note>& lane, int LaneNum, string songPath, int diffNumber, int bpm) {
	string fPath = songPath + "charts/" + to_string(diffNumber) + "/L" + to_string(LaneNum) + ".txt";
	inputFile.open(fPath);

	string line = "";
	const char delim = ':';
	if (inputFile) {
		while (inputFile >> line) {
			std::vector<std::string> out;
			tokenize2(line, delim, out);

			bool hold = false;
			if (out[1] == "1") {
				hold = true;
			}

			Note temp(stof(out[0]), hold, stof(out[2]), stoi(out[3]), bpm);
			
			lane.push_back(temp);
		}
	}
	inputFile.close();
	logger.log(L"Lane " + to_wstring(LaneNum) + L" is ready.");
}

/**
 * Readin the wheel notes from a file.
 * 
 * @param wheel WheelNote vector to store the notes in
 * @param songPath path of the song
 * @param diffNumber difficulty of the song
 * @param bpm BPM of the song
 */
void parseInWheel(vector<WheelNote>& wheel, string songPath, int diffNumber, int bpm) {
	string fPath = songPath + "charts/" + to_string(diffNumber) + "/Wheel.txt";
	inputFile.open(fPath);

	string line = "";
	const char delim = ':';
	if (inputFile) {
		while (inputFile >> line) {
			std::vector<std::string> out;
			tokenize2(line, delim, out);

			bool slam = false;
			if (out[1] == "1") {
				slam = true;
			}
			
			WheelNote temp(stof(out[0]), slam, stoi(out[2]), stoi(out[3]), stoi(out[4]), stof(out[5]), stoi(out[6]), bpm);
			
			wheel.push_back(temp);
		}
	}
	inputFile.close();
	logger.log(L"Lane Wheel is Ready.");
}

/**
 * Split the string based on the delim char.
 * 
 * @param str Original String
 * @param delim char to split based on
 * @param out Vector string split into
 */
void tokenize2(std::string const& str, const char delim, std::vector<std::string>& out) {

	size_t start;
	size_t end = 0;

	while ((start = str.find_first_not_of(delim, end)) != std::string::npos)
	{
		end = str.find(delim, start);
		out.push_back(str.substr(start, end - start));
	}
}

/**
 * Update when the notes appear on screen.
 * 
 * @param lane Lane to adjust the notes of
 */
void updateAppearTime(vector<Note>& lane) {
	for (size_t i = 0; i < lane.size(); i++) {
		lane[i].speedChangeAppearTime(gameState.getSpeed());
	}
}

void updateAppearTime(vector<WheelNote>& lane) {
	for (size_t i = 0; i < lane.size(); i++) {
		lane[i].speedChangeAppearTime(gameState.getSpeed());
	}
}

/**
 * Calculate the distance of the note.
 * 
 * @param distance
 */
void updateScreenTime(float& distance) {
	distance = (float)(453075.9705 / pow(gameState.getSpeed(), 1.004140998));
}

/**
 * Draw the notes for the wheel.
 * 
 * @param wheel the vector holding the wheel notes
 * @param currentSongOffset current time in the song
 * @param missCount reference to the count of missed notes in this song																										
 */
void GameRenderer::drawWheelNotes(vector<WheelNote>& wheel, std::chrono::milliseconds currentSongOffset, int& missCount, float distance) {
	// First check if the lane is empty
	if (wheel.empty()) {
		return;
	}

	// Draw the wheel notes on screen
	for (size_t i = 0; i < wheel.size(); i++) {
		// Check if on screen yet
		if (wheel[i].appearTime <= ((float)currentSongOffset.count())) {

			// Calculate the yPosition for the note regardless of type
			float yPos = 1080.f - (DISTANCE_TO_PERFECT * ((currentSongOffset.count() - wheel[i].appearTime) / distance));

			if (wheel[i].isSlam()) {
				// SLAM NOTES
				
				if (wheel[i].getDirection() == -1) {
					// Slam Left
					
					// Reset the note instance's position
					wheelSlamLeft->reset();

					// Apply the base transformations
					wheelSlamLeft->scale(1.f, laneNoteScale * 0.159f * 2.63636363f, laneNoteScale);

					// Set the Y position
					wheelSlamLeft->setPositionY(yPos);

					// Set Opacity
					wheelSlamLeft->setOpacity(0.5f);
					
					// Draw the note
					wheelSlamLeft->render(PROJECTION::PERSPECTIVE);
				}
				else {
					// Slam Right

					// Reset the note instance's position
					wheelSlamRight->reset();

					// Apply the base transformations
					wheelSlamRight->scale(1.f, laneNoteScale * 0.159f * 2.63636363f, laneNoteScale);

					// Set the Y position
					wheelSlamRight->setPositionY(yPos);

					// Set Opacity
					wheelSlamRight->setOpacity(0.5f);

					// Draw the note
					wheelSlamRight->render(PROJECTION::PERSPECTIVE);
				}
			}
			else {
				// CONTINUOUS NOTES
				// ** Compute X values **
				// Note width is 66 pixels, lanes are 74 pixels apart
				float xStartLeft = ((float)wheel[i].getStartPos() - 3.f) * 74.f - 34.5f;
				float xStartRight = xStartLeft + 69.f;

				float xEndLeft = ((float)wheel[i].getEndPos() - 3.f) * 74.f - 34.5f;
				float xEndRight = xEndLeft + 69.f;

				// ** Compute Y Height **
				// 1. Calculate the number of milliseconds from the top of the screen to the perfect line
				float millisecondsOnScreen = 453075.9705f / pow((float)gameState.getSpeed(), 1.004140998f); // ms

				// 2. Now divide that by the distance to the perfect line to get the number of pixels per millisecond on screen
				// The 1040.25 is calculated from: 1080.f - 921.f = 159.f / 4.f = 39.75f * 3.f = 119.25 + 921.f = 1040.25f
				// **Not sure why that is the number we needed, but it is...**
				float pixelsPerMillisecond = 1040.25f / millisecondsOnScreen; // px/ms

				// 3. Now we calculate total pixels for that note
				float noteSizeInPixels = wheel[i].getWheelNoteLength() * pixelsPerMillisecond; // px

				// Set vertices and render
				wheelPixelNote->setVertexPixelLocation(0, xStartLeft, -11.f);
				wheelPixelNote->setVertexPixelLocation(1, xStartRight, -11.f);
				wheelPixelNote->setVertexPixelLocation(2, xEndRight, noteSizeInPixels + 11.f);
				wheelPixelNote->setVertexPixelLocation(3, xEndLeft, noteSizeInPixels + 11.f);
				wheelPixelNote->setPositionY(yPos);

				wheelPixelNote->setOpacity(0.5f);

				wheelPixelNote->render(PROJECTION::PERSPECTIVE);
			}
		}
	}
}

/**
 * Draw the notes for a lane.
 * 
 * @param laneNum the current lane number
 * @param lane the vector holding the notes
 * @param currentSongOffset current time in the song
 * @param missCount reference to the count of missed notes in this song
 */
void GameRenderer::drawLaneNotes(int laneNum, vector<Note> &lane, std::chrono::milliseconds currentSongOffset, int &missCount, float distance) {
	// First check if the lane is empty
	if (lane.empty()) {
		return;
	}

	for (size_t i = 0; i < lane.size(); i++) {
		// Check if on screen yet
		if (lane[i].appearTime <= ((float)currentSongOffset.count())) {

			// Calculate the yPosition for the note regardless of type
			float yPos = 1080.f - (DISTANCE_TO_PERFECT * ((currentSongOffset.count() - lane[i].appearTime) / distance));

			if (lane[i].isHold()) {
				// ** Compute X values **
				// Note width is 66 pixels, lanes are 74 pixels apart
				float xLeft = (laneNum - 3.f) * 74.f - 34.5f;
				float xRight = xLeft + 69.f;

				// ** Compute Y Height **
				// 1. Calculate the number of milliseconds from the top of the screen to the perfect line
				float millisecondsOnScreen = 453075.9705f / pow((float)gameState.getSpeed(), 1.004140998f); // ms

				// 2. Now divide that by the distance to the perfect line to get the number of pixels per millisecond on screen
				// The 1040.25 is calculated from: 1080.f - 921.f = 159.f / 4.f = 39.75f * 3.f = 119.25 + 921.f = 1040.25f
				// **Not sure why that is the number we needed, but it is...**
				float pixelsPerMillisecond = 1040.25f / millisecondsOnScreen; // px/ms

				// 3. Now we calculate total pixels for that note
				float noteSizeInPixels = lane[i].getHoldLength() * pixelsPerMillisecond; // px

				// Set vertices and render
				holdPixelNote->setVertexPixelLocation(0, xLeft, -11.f);
				holdPixelNote->setVertexPixelLocation(1, xRight, -11.f);
				holdPixelNote->setVertexPixelLocation(2, xRight, noteSizeInPixels + 11.f);
				holdPixelNote->setVertexPixelLocation(3, xLeft, noteSizeInPixels + 11.f);

				holdPixelNote->setPositionY(yPos);

				// Store that yPos onto the note itself for input calculations
				lane[i].setYPos(yPos);

				holdPixelNote->render(PROJECTION::PERSPECTIVE);
			}
			else {
				// REGULAR NOTES

				// Reset the note instance's position
				laneNote->reset();

				// Apply the base transformations
				laneNote->scale(laneNoteScale, laneNoteScale * 0.159f, laneNoteScale);

				// Translate the the x lane position
				switch (laneNum) {
					case 1:
						laneNote->translate(-0.4f, 0.f, 0.f);
						break;
					case 2:
						laneNote->translate(-0.2f, 0.f, 0.f);
						break;
					case 4:
						laneNote->translate(0.2f, 0.f, 0.f);
						break;
					case 5:
						laneNote->translate(0.4f, 0.f, 0.f);
						break;
					case 3:
					default:
						break;
				}

				// Set the Y position
				laneNote->setPositionY(yPos);

				// Store that yPos onto the note itself for input calculations
				lane[i].setYPos(yPos);

				// Draw the note
				laneNote->render(PROJECTION::PERSPECTIVE);
			}
		}
	}
}

wstring getScoreString(float score) {
	wstring scoreS = to_wstring((int)score);
	while (scoreS.length() < 7) {
		scoreS = L"0" + scoreS;
	}
	return scoreS;
}

void GameRenderer::drawJudgement(JUDGEMENT judgement, std::chrono::milliseconds currentSongOffset, float& clearTime) {
	// Number of milliseconds to keep the judgement on screen before clearing it
	float timeOnScreen = 750.f;

	// Set the sprite to use that graphic
	switch (judgement) {
		case JUDGEMENT::PERFECT_HIT:
			noteJudgement->setTextureID(TextureList::Inst()->GetTextureID("Textures/Judgement/temp_Perfect.png"));
			break;
		case JUDGEMENT::NEAR_HIT:
			noteJudgement->setTextureID(TextureList::Inst()->GetTextureID("Textures/Judgement/temp_Near.png"));
			break;
		case JUDGEMENT::MISS:
			noteJudgement->setTextureID(TextureList::Inst()->GetTextureID("Textures/Judgement/temp_Miss.png"));
			break;
	}

	// Set the clearTime for use in the render loop
	clearTime = currentSongOffset.count() + timeOnScreen;
}

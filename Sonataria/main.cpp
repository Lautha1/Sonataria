#include "ControllerInput.h"
#include "GameState.h"
#include <iostream>
#include "Networking.h"
#include <PacDrive/PacDrive.h>
#include "ScreenRenderer.h"
#include <SFML/Graphics.hpp>
#include <Windows.h>
#include "WindowsAudio.h"
#include "UserData.h"
#include "Logger.h"
#include "RFIDCardReader.h"
#include <GL/glew.h>
#include <sstream>

//Forward Declarations
void renderingThread(sf::RenderWindow* window);
void resetAll();
void networkCheckingThread();

// Hold the copy of the screen renderer
ScreenRenderer screenRenderer;

void GLAPIENTRY MessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity,
	GLsizei length, const GLchar* message, const void* userParam) {
	// Turn source into a string
	string sourceStr = "unknown";
	switch (source) {
		case GL_DEBUG_SOURCE_API: sourceStr = "OpenGL API"; break;
		case GL_DEBUG_SOURCE_WINDOW_SYSTEM: sourceStr = "Window System"; break;
		case GL_DEBUG_SOURCE_SHADER_COMPILER: sourceStr = "Shader Compiler"; break;
		case GL_DEBUG_SOURCE_THIRD_PARTY: sourceStr = "3rd Party"; break;
		case GL_DEBUG_SOURCE_APPLICATION: sourceStr = "Application"; break;
		case GL_DEBUG_SOURCE_OTHER: sourceStr = "Other"; break;
		default: break;
	}

	// Turn type into a string
	string typeStr = "unknown";
	switch (type) {
		case GL_DEBUG_TYPE_ERROR: typeStr = "Error"; break;
		case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: typeStr = "Deprecated Behavior"; break;
		case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: typeStr = "Undefined Behavior"; break;
		case GL_DEBUG_TYPE_PORTABILITY: typeStr = "Portability"; break;
		case GL_DEBUG_TYPE_PERFORMANCE: typeStr = "Performance"; break;
		case GL_DEBUG_TYPE_MARKER: typeStr = "Marker"; break;
		case GL_DEBUG_TYPE_PUSH_GROUP: typeStr = "Push Group"; break;
		case GL_DEBUG_TYPE_POP_GROUP: typeStr = "Pop Group"; break;
		case GL_DEBUG_TYPE_OTHER: typeStr = "Other"; break;
		default: break;
	}

	// Turn severity into a string
	string severityStr = "unknown";
	switch (severity) {
		case GL_DEBUG_SEVERITY_LOW: severityStr = "Low"; break;
        case GL_DEBUG_SEVERITY_MEDIUM: severityStr = "Medium"; break;
        case GL_DEBUG_SEVERITY_HIGH: severityStr = "High"; break;
		case GL_DEBUG_SEVERITY_NOTIFICATION: severityStr = "Notification"; break;
		default: break;
	}

	// Ouptut the error to 'stderr'
	fprintf(stderr, "GL DEBUG (%s/%s/%s): [%04d] %s\n",
		sourceStr.c_str(), typeStr.c_str(), severityStr.c_str(),
		id, message);
}

/**
 * Main method.
 * 
 * @return exit code
 */
int main(int argc, char** argv) {
	
	// Declare the window to be used
	sf::ContextSettings mySettings = sf::ContextSettings();
	mySettings.antialiasingLevel = 4;
	sf::RenderWindow gameWindow(sf::VideoMode(1920, 1080), "Sonataria", sf::Style::Fullscreen, mySettings);

	// Enable Glew
	glewExperimental = GL_TRUE;
	if (GLEW_OK != glewInit()) {
		logger.logError(L"Failed to initialize GLEW");
		exit(EXIT_FAILURE);
	} else {
		logger.log(L"GLEW initialized successfully!");

		// Output some GLEW/OpenGL data
		std::wstringstream outputBuilder;
		outputBuilder << L" > OpenGL Version:  " << (const char*)glGetString(GL_VERSION);
		logger.log(outputBuilder.str());

		outputBuilder.str(L"");
		outputBuilder << L" > GLSL Version:    " << (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION);
		logger.log(outputBuilder.str());

		outputBuilder.str(L"");
		outputBuilder << L" > OpenGL Vendor:   " << (const char*)glGetString(GL_VENDOR);
		logger.log(outputBuilder.str());

		outputBuilder.str(L"");
		outputBuilder << L" > OpenGL Renderer: " << (const char*)glGetString(GL_RENDERER);
		logger.log(outputBuilder.str());

		// Enable OpenGL Debug logging
		glEnable(GL_DEBUG_OUTPUT);
		glDebugMessageCallback(MessageCallback, 0);

		// Turn off logging of 'notification' messages
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, NULL, GL_FALSE);
	}

	// Set some stuff for the window
	gameWindow.setKeyRepeatEnabled(false);
	gameWindow.setMouseCursorVisible(false);
	gameWindow.setActive(false);

	// Initialize the LED Controller
	PacInitialize();
	PacSetLEDStates(0, 0x0);

	// Begin the rendering thread
	sf::Thread thread(&renderingThread, &gameWindow);
	thread.launch();

	// Set the default position of the wheel (mouse)
	sf::Mouse::setPosition(sf::Vector2i(1, 0));

	// Start Timer loop to check network status 
	sf::Thread updateTimerThread(&networkCheckingThread);
	updateTimerThread.launch();

	// Used to poll the window for events
	while (gameWindow.isOpen()) {
		RFIDCardReader::getCardReader()->poll();

		sf::Event evnt;
		while (gameWindow.pollEvent(evnt)) {
			if (evnt.type == sf::Event::Closed) {
				gameWindow.close();
			}

			RFIDCardReader::getCardReader()->poll();

			 //Left Btn - A - 1
			 //Left Mid Btn - C - 2
			 //Center Btn - B - 3
			 //Right Mid Btn - M - 4
			 //Right Btn - L - 5
			 //Start Btn - T - 6

			 // Key press events
			 if (evnt.type == sf::Event::KeyPressed) {
				 if (evnt.key.code == sf::Keyboard::A) {
					 controllerInput.setKeyState(1, true);
					 PacSetLEDState(0, 5, true);
				 } 
				 else if(evnt.key.code == sf::Keyboard::C) {
					 controllerInput.setKeyState(2, true);
					 PacSetLEDState(0, 4, true);
					 if (gameState.getGameState() == GameState::CurrentState::TEST_MENU_MAIN) {
						 screenRenderer.testMenuPosPlus();
					 } 
					 else if (gameState.getGameState() == GameState::CurrentState::TEST_MENU_IOCHECK) {
						 screenRenderer.testMenuIOCheckPosPlus();
					 }
					 else if (gameState.getGameState() == GameState::CurrentState::TEST_MENU_SOUNDOPTIONS) {
						 if (!screenRenderer.getTestMenuSoundOptionsSelected()) {
							 screenRenderer.testMenuSoundOptionsPosPlus();
						 }
					 }
					 else if (gameState.getGameState() == GameState::CurrentState::TEST_MENU_NETWORKING) {
						 screenRenderer.testMenuNetworkingPosPlus();
					 }
				 } 
				 else if(evnt.key.code == sf::Keyboard::B) {
					 controllerInput.setKeyState(3, true);
					 PacSetLEDState(0, 2, true);
				 } 
				 else if(evnt.key.code == sf::Keyboard::M) {
					 controllerInput.setKeyState(4, true);
					 PacSetLEDState(0, 1, true);
					 if (gameState.getGameState() == GameState::CurrentState::TEST_MENU_MAIN) {
						 screenRenderer.testMenuPosMinus();
					 }
					 else if (gameState.getGameState() == GameState::CurrentState::TEST_MENU_IOCHECK) {
						 screenRenderer.testMenuIOCheckPosMinus();
					 }
					 else if (gameState.getGameState() == GameState::CurrentState::TEST_MENU_SOUNDOPTIONS) {
						 if (!screenRenderer.getTestMenuSoundOptionsSelected()) {
							 screenRenderer.testMenuSoundOptionsPosMinus();
						 }
					 }
					 else if (gameState.getGameState() == GameState::CurrentState::TEST_MENU_NETWORKING) {
						 screenRenderer.testMenuNetworkingPosMinus();
					 }
					 else if (gameState.getGameState() == GameState::CurrentState::SONG_SELECT) {
						 screenRenderer.changeDifficultySelected(1);
					 }
				 } 
				 else if(evnt.key.code == sf::Keyboard::L) {
					 controllerInput.setKeyState(5, true);
					 PacSetLEDState(0, 0, true);
				 } 
				 else if(evnt.key.code == sf::Keyboard::T) {
					 controllerInput.setKeyState(6, true);
					 PacSetLEDState(0, 3, true);
					 if (gameState.getGameState() == GameState::CurrentState::TEST_MENU_MAIN) {
						 switch (screenRenderer.getTestMenuPos()) {
							 case 0:
								 gameState.setGameState(GameState::CurrentState::TEST_MENU_IOCHECK);
								 screenRenderer.testMenuReset();
								 break;
							 case 1:
								 gameState.setGameState(GameState::CurrentState::TEST_MENU_SOUNDOPTIONS);
								 screenRenderer.testMenuReset();
								 break;
							 case 2:
								 gameState.setGameState(GameState::CurrentState::TEST_MENU_NETWORKING);
								 screenRenderer.testMenuReset();
								 break;
							 case 3:
								 gameState.setGameState(GameState::CurrentState::TEST_MENU_SYSINFO);
								 screenRenderer.testMenuReset();
								 break;
							 case 4:
								 gameState.setGameState(GameState::CurrentState::TITLE_SCREEN);
								 screenRenderer.testMenuReset();
								 break;
						 }
					 }
					 else if (gameState.getGameState() == GameState::CurrentState::TEST_MENU_IOCHECK) {
						 switch (screenRenderer.getTestMenuIOCheckPos()) {
							 case 0:
								 gameState.setGameState(GameState::CurrentState::TEST_MENU_INPUTCHECK);
								 screenRenderer.testMenuReset();
								 break;
							 case 1:
								 gameState.setGameState(GameState::CurrentState::TEST_MENU_MAIN);
								 screenRenderer.testMenuReset();
								 break;
						 }
					 }
					 else if (gameState.getGameState() == GameState::CurrentState::TEST_MENU_INPUTCHECK) {
						 gameState.setGameState(GameState::CurrentState::TEST_MENU_IOCHECK);
						 screenRenderer.testMenuReset();
					 }
					 else if (gameState.getGameState() == GameState::CurrentState::TEST_MENU_SYSINFO) {
						 gameState.setGameState(GameState::CurrentState::TEST_MENU_MAIN);
						 screenRenderer.testMenuReset();
					 }
					 else if (gameState.getGameState() == GameState::CurrentState::TEST_MENU_SOUNDOPTIONS) {
						 switch (screenRenderer.getTestMenuSoundOptionsPos()) {
							 case 0: // System Volume
								 if (screenRenderer.getTestMenuSoundOptionsSelected()) {
									 screenRenderer.setTestMenuSoundOptionsSelected(false);
								 }
								 else {
									 screenRenderer.setTestMenuSoundOptionsSelected(true);
								 }
								 break;
							 case 1: // Return to previous menu
								 gameState.setGameState(GameState::CurrentState::TEST_MENU_MAIN);
								 screenRenderer.testMenuReset();
								 break;
						 }
					 }
					 else if (gameState.getGameState() == GameState::CurrentState::TEST_MENU_NETWORKING) {
						 int status = 0;
						 switch (screenRenderer.getTestMenuNetworkingPos()) {
							 case 0: // Network check
								 logger.log(L"Testing Network...");
								 screenRenderer.isNetworkChecking = true;
								 status = network.checkConnection();

								 switch (status) {
									 case 1:
										 gameState.setOnlineState(GameState::OnlineState::ONLINE);
										 break;
									 case 2:
										 gameState.setOnlineState(GameState::OnlineState::OFFLINE);
										 break;
									 case 3:
										 gameState.setOnlineState(GameState::OnlineState::MAINTENENCE);
										 break;
								 }
								 screenRenderer.isNetworkChecking = false;
								 logger.log(L"Network Test Finished.");
								 break;
							 case 1: // Move back back a menu
								 gameState.setGameState(GameState::CurrentState::TEST_MENU_MAIN);
								 screenRenderer.testMenuReset();
								 break;
						 }
					 }
					 else if (gameState.getGameState() == GameState::CurrentState::TITLE_SCREEN) {
						 RFIDCardReader::getCardReader()->clearLastCardData();
						 gameState.setGameState(GameState::CurrentState::PRELOGIN);
						 controllerInput.reset();
					 }
					 else if (gameState.getGameState() == GameState::CurrentState::PRELOGIN) {
						 // TODO: go to tutorial and such...
						 gameState.setGameState(GameState::CurrentState::SONG_SELECT);
						 controllerInput.reset();
					 }
					 else if (gameState.getGameState() == GameState::CurrentState::SONG_SELECT) {
						 if (screenRenderer.isCurrentSongValidToPlay()) {
							 gameState.setSongPlaying(screenRenderer.currentPageSongs[screenRenderer.getSongSelectHoverOver()], screenRenderer.currentPageSongs[screenRenderer.getSongSelectHoverOver()].getDifficultyNumber(screenRenderer.getDifficultyHoverOver()));
							 gameState.setGameState(GameState::CurrentState::GAME);
						 }
						 else {
							 // TODO: Play the invalid selection sound effect
						 }
						 controllerInput.reset();
					 }
					 else if (gameState.getGameState() == GameState::CurrentState::RESULTS) {
						 if (gameState.results.size() >= 2) {
							 // TODO: Check for EX Track then go to final results or ex track
							 gameState.setGameState(GameState::CurrentState::FINAL_RESULTS);
						 }
						 else {
							 gameState.setGameState(GameState::CurrentState::SONG_SELECT);
						 }
						 controllerInput.reset();
					 }
					 else if (gameState.getGameState() == GameState::CurrentState::FINAL_RESULTS) {
						 gameState.setGameState(GameState::CurrentState::TITLE_SCREEN);
						 // Clear the saved data at the end of the credit regardless of whether or not they are carded in
						 userData.clearData();
					 }
				 }
				 else if(evnt.key.code == sf::Keyboard::Escape) {
					 logger.log(L"Shutting down from ESC key");
					 gameState.setGameState(GameState::CurrentState::SHUTDOWN);
					 thread.wait();
					 controllerInput.reset();
					 PacShutdown();
					 exit(0);
				 }
			 }
			 
			 // Key Released Events
			 if (evnt.type == sf::Event::KeyReleased) {
				 if (evnt.key.code == sf::Keyboard::A) {
					 controllerInput.setKeyState(1, false);
					 PacSetLEDState(0, 5, false);
				 } 
				 else if(evnt.key.code == sf::Keyboard::C) {
					 controllerInput.setKeyState(2, false);
					 PacSetLEDState(0, 4, false);
				 } 
				 else if(evnt.key.code == sf::Keyboard::B) {
					 controllerInput.setKeyState(3, false);
					 PacSetLEDState(0, 2, false);
				 } 
				 else if(evnt.key.code == sf::Keyboard::M) {
					 controllerInput.setKeyState(4, false);
					 PacSetLEDState(0, 1, false);
				 } 
				 else if(evnt.key.code == sf::Keyboard::L) {
					 controllerInput.setKeyState(5, false);
					 PacSetLEDState(0, 0, false);
				 } 
				 else if(evnt.key.code == sf::Keyboard::T) {
					 controllerInput.setKeyState(6, false);
					 PacSetLEDState(0, 3, false);
				 }
			 }

			 // Mouse Moved Events
			 if (evnt.type == sf::Event::MouseMoved) {
				 if (gameState.getGameState() != GameState::CurrentState::TITLE_SCREEN) {
					 if (sf::Mouse::getPosition().x > 1) {
						 controllerInput.changeWheelPos(1);
						 if (gameState.getGameState() == GameState::CurrentState::SONG_SELECT) {
							 screenRenderer.updateWheelRelation(1);
						 }
						 else if (gameState.getGameState() == GameState::CurrentState::GAME) {
							 if (controllerInput.getKeyboardState().getKeyState(6) == true) {
								 //If in the game and holding down the start button (increase x)
								 int newSpeed = gameState.getSpeed() + 1;
								 if (newSpeed > 999) {
									 newSpeed = 999;
								 }
								 gameState.setSpeed(newSpeed);
							 }
						 }
						 else if (gameState.getGameState() == GameState::CurrentState::TEST_MENU_SOUNDOPTIONS) {
							 if (screenRenderer.getTestMenuSoundOptionsSelected()) {
								 switch (screenRenderer.getTestMenuSoundOptionsPos()) {
								 case 0:
									 windowsAudio.SetSystemVolume((float)(windowsAudio.GetSystemVolume(WindowsAudio::VolumeUnit::Scalar) + 0.01), WindowsAudio::VolumeUnit::Scalar);
									 break;
								 }
							 }
						 }
					 }
					 else if (sf::Mouse::getPosition().x < 1) {
						 controllerInput.changeWheelPos(-1);
						 if (gameState.getGameState() == GameState::CurrentState::SONG_SELECT) {
							 screenRenderer.updateWheelRelation(-1);
						 }
						 else if (gameState.getGameState() == GameState::CurrentState::GAME) {
							 if ((controllerInput.getKeyboardState().getKeyState(6) == true)) {
								 //If in the game and holding down the start button (decrease x)
								 int newSpeed = gameState.getSpeed() - 1;
								 if (newSpeed < 100) {
									 newSpeed = 100;
								 }
								 gameState.setSpeed(newSpeed);
							 }
						 }
						 else if (gameState.getGameState() == GameState::CurrentState::TEST_MENU_SOUNDOPTIONS) {
							 if (screenRenderer.getTestMenuSoundOptionsSelected()) {
								 switch (screenRenderer.getTestMenuSoundOptionsPos()) {
									 case 0:
										 windowsAudio.SetSystemVolume((float)(windowsAudio.GetSystemVolume(WindowsAudio::VolumeUnit::Scalar) - 0.01), WindowsAudio::VolumeUnit::Scalar);
										 break;
								 }
							 }
						 }
					 }
					 sf::Mouse::setPosition(sf::Vector2i(1, 0));
				 }
			 }

			 //TEST BUTTON PRESSED
			 if (evnt.type == sf::Event::KeyPressed && evnt.key.code == sf::Keyboard::Period) {
				 
				 logger.log(L"TEST Button Pressed - Attempting to stop render thread.");
				 logger.log(L"Controls Disabled until Test Menu Opened.");
				 gameState.setServicePressed(true);
				 thread.wait();

				 logger.log(L"Thread Finished - Changing Game State to Test Menu.");
				 gameState.setServicePressed(false);
				 gameState.setGameState(GameState::CurrentState::TEST_MENU_MAIN);
				 resetAll();
				 screenRenderer.testMenuReset();

				 logger.log(L"Relaunching Thread - Into Test Menu Game State.");
				 thread.launch();
			 }

			 // Mostly used when the update finishes downloading
			 if (gameState.getGameState() == GameState::CurrentState::SHUTDOWN) {
				 thread.wait();
				 controllerInput.reset();
				 PacShutdown();
				 exit(0);
			 }
		}
	}

	return 0;
}

/**
 * Start the render window thread.
 * 
 * @param window the render window
 */
void renderingThread(sf::RenderWindow* window) {
	screenRenderer.render(window);
}

/**
 * Loop to check the network every 30 seconds on the title screen.
 * 
 */
void networkCheckingThread() {
	while (true) {
		std::this_thread::sleep_for(std::chrono::milliseconds(30000));
		// Only check the network while on loop while on title screen
		// Save scores and other things try to connect on their own
		if (gameState.getGameState() == GameState::CurrentState::TITLE_SCREEN) {
			int result = network.checkConnection();
			//1-online 2-offline 3-maint

			switch (result) {
				case 1:
					gameState.setOnlineState(GameState::OnlineState::ONLINE);
					break;
				case 2:
					gameState.setOnlineState(GameState::OnlineState::OFFLINE);
					break;
				case 3:
					gameState.setOnlineState(GameState::OnlineState::MAINTENENCE);
					break;
			}
			logger.log(L"SET ONLINE STATE TO: [" + to_wstring(result) + L"]");
		}
	}
}

/**
 * Reset the game state, screen renderer and controller input.
 * 
 */
void resetAll() {
	gameState.resetResults();
	screenRenderer.reset();
	controllerInput.reset();
}
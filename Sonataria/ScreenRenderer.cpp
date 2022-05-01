#include <chrono>
#include "ControllerInput.h"
#include <filesystem>
#include <fstream>
#include "GameState.h"
#include "GameRenderer.h"
#include "Logger.h"
#include "Networking.h"
#include "RFIDCardReader.h"
#include "ScreenRenderer.h"
#include "SoundEffects.h"
#include "SystemSettings.h"
#include <tchar.h>
#include "TextureList.h"
#include "unzip.h"
#include "UserData.h"
#include "WindowsAudio.h"
using namespace std;

// Holds all the song paths until all of the songs are loaded
vector<string> songPaths;

// How many menu spots for songs (real + fake) will be needed
int songSpots = 0;

// Variables used for startup
bool allChecksComplete = false;

// String to hold the new version of the game if an update is detected
string newVersion;

/* NETWORK STATUS
*	-1 - Not started to check
*   0  - Checking
*	1  - ONLINE/CAN CONNECT
*	2  - OFFLINE/COULDN'T CONNECT
*	3  - MAINT
*/
int networkStatus = -1;

/* UPDATE CHECK STATUS
*	-2 - Not started to check
*   -1 - Skipped
*   0  - Checking
*	1  - UPDATE AVAILABLE
*	2  - NO UPDATE
*/
int updateCheckStatus = -2;

/* UPDATE DOWNLOAD STATUS
*	-1 - Not started
*	 0 - Downloading
*	 1 - Extracting
*	 2 - Modifying Startup
*	 3 - Restarting
*/
int updateDownloadStatus = -1;

// Screen Size Ratio;
const float aspect = 16.f / 9.f;

// Forward declarations
void execStartupChecks();
void execDownloadUpdate();
void rThread(sf::RenderWindow* window);
void checkForProfile();

/**
 * Constructor for the screen renderer.
 */
ScreenRenderer::ScreenRenderer() {

	logger.log(L"Beginning Startup for Sonataria");

	// Need to make sure that the game can delete stuff if needed (allows time for the old version to close after an update)
	std::chrono::milliseconds timespan(1000);
	this_thread::sleep_for(3 * timespan);

	logger.log(L"Checking for old versions");
	// Check for old versions of the game and delete them
	ifstream inFile;
	inFile.open("C:/old_version.txt");
	if (!inFile.is_open()) {
		logger.logError(L"Old version file NOT detected!");
	}
	else {
		string contents;
		inFile >> contents;
		if (contents == "" || contents == " ") {
			logger.log(L"No old versions detected.");
		}
		else {
			string oldPath = "C:/" + contents;

			uintmax_t n = filesystem::remove_all(oldPath);
			logger.log(L"Cleaned up " + to_wstring(n) + L" files from the old version.");

			std::ofstream ofs("C:/old_version.txt", std::ios::out | std::ios::trunc); // clear contents
			ofs.close();

			filesystem::remove("C:/SNA_UPDATE/Update.zip");

			logger.log(L"Cleaned up new version zip file.");
		}
	}

	this->testMenuPos = 0;
	this->testMenuTotalOptions = 5;
	this->testMenuIOChkPos = 0;
	this->testMenuIOCheckTotalOptions = 2;
	this->testMenuSoundOptionsPos = 0;
	this->testMenuSoundOptionsTotalOptions = 2;
	this->testMenuSoundOptionsSelected = false;
	this->testMenuNetworkingPos = 0;
	this->testMenuNetworkingTotalOptions = 2;
	this->songSelectHoverOver = 0;
	this->wheelRelation = 25;
	this->currentSongPage = 1;
	this->difficultyHoverOver = 0;
	this->isNetworkChecking = false;
	this->openGLInitialized = false;

	this->loginComplete = -1;

	// SETUP SPRITES
	
	// General Sprites
	Stage = new QuadSprite(L"Stage");
	OpenCurtains = new QuadSprite(L"Open Curtains");
	ClosedCurtainLeft = new QuadSprite(L"Closed Curtain Left");
	ClosedCurtainRight = new QuadSprite(L"Closed Curtain Right");
	SpotlightLeft = new QuadSprite(L"Spotlight Left");
	SpotlightRight = new QuadSprite(L"Spotlight Right");
	CurvySymbol = new QuadSprite(L"Curvy Symbol");

	// Login
	TapLifeLinkPass = new QuadSprite(L"Tap Life Link Pass");
	OR = new QuadSprite(L"OR");
	BeginAsGuest = new QuadSprite(L"Begin As Guest");
	PosterA = new QuadSprite(L"Poster A");
	PosterB = new QuadSprite(L"Poster B");

	// UI Elements
	Frame = new QuadSprite(L"Frame");

	// Backgrounds
	AttractTitleScreen = new VideoSprite(L"Attract Title Screen Video");
	SetMorning = new QuadSprite(L"Set Morning");
	SetAfternoon = new QuadSprite(L"Set Afternoon");

	// PreLogin Sprites
	LifeLinkIcon = new QuadSprite(L"Life Link Icon");

	// Playbills
	Act1 = new QuadSprite(L"Act 1");
	Act1Fin = new QuadSprite(L"Act 1 Fin");
	Act2 = new QuadSprite(L"Act 2");
	Act2Fin = new QuadSprite(L"Act 2 Fin");
	Fin = new QuadSprite(L"Fin");
	PreSelectAllBoxes = new QuadSprite(L"PreSelect All Boxes");

	JacketArt = new QuadSprite(L"Jacket Art");

	std::string dir = "./Songs";

	logger.log(L"Reading in song paths...");

	// Read in Songs (looks for info.txt files)
	for (auto& item : filesystem::recursive_directory_iterator(dir)) {

		if (!filesystem::is_regular_file(item.path())
			|| item.path().extension() != ".json")
			continue;

		if (item.path().filename() != "info.json") {
			continue;
		}

		logger.log(L"Found song at: " + item.path().wstring());
		songPaths.push_back(item.path().string());
	}

	logger.log(L"Total Song Count: " + to_wstring(songPaths.size()));

	// Calculate how many songs are missign to make the total a multiple of 6
	int remainder = (6 - (songPaths.size() % 6));
	if (remainder == 6) {
		remainder = 0;
	}

	songSpots = songPaths.size() + remainder;

	logger.log(L"Song Spots Needed: " + to_wstring(songSpots));

	// In the array of song paths, replace some characters
	for (size_t i = 0; i < songPaths.size(); i++) {
		replace(songPaths[i].begin(), songPaths[i].end(), '\\', '/');
	}

	// Add in the songs that are real to the songs vector
	for (size_t i = 0; i < songPaths.size(); i++) {
		Song temp(songPaths[i]);
		songs.push_back(temp);
	}

	logger.log(L"Creating " + to_wstring(remainder) + L" dummy songs.");

	// Adds in the number of dummy songs to make a full 6 roster
	Song temp;
	for (int i = 0; i < remainder; i++) {
		songs.push_back(temp);
	}

	logger.log(L"Size of Songs Vector: " + to_wstring(songs.size()));

	// Set the number of pages of songs
	this->numOfSongPages = songSpots / 6;

	// Add the first 6 songs to setup the first page of songs
	for (int i = 0; i < 6; i++) {
		currentPageSongs.push_back(songs[i]);
	}

	logger.log(L"Starting to load in saved settings.");

	systemSettings.initSettings();
}

/**
 * Default deconstructor.
 */
ScreenRenderer::~ScreenRenderer() {
	// DELETE SPRITES
	
	// General Sprites
	delete Stage;
	delete OpenCurtains;
	delete ClosedCurtainLeft;
	delete ClosedCurtainRight;
	delete SpotlightLeft;
	delete SpotlightRight;
	delete CurvySymbol;

	// Login
	delete TapLifeLinkPass;
	delete OR;
	delete BeginAsGuest;
	delete PosterA;
	delete PosterB;

	// UI Elements
	delete Frame;

	// Backgrounds
	delete AttractTitleScreen;
	delete SetMorning;
	delete SetAfternoon;

	// PreLogin Sprites
	delete LifeLinkIcon;

	// Playbills
	delete Act1;
	delete Act1Fin;
	delete Act2;
	delete Act2Fin;
	delete Fin;
	delete PreSelectAllBoxes;

	delete JacketArt;

	// DELETE TEXTS
	delete DisplayName;
	delete UserTitle;
	delete Level;
	delete PlayCount;
}

/**
 * Render the screen.
 *
 * @param gameWindow the window to render to
 */
void ScreenRenderer::render(sf::RenderWindow* gameWindow) {

	logger.log(L"Screen Renderer Now Active.");

	// Set the game window to active
	gameWindow->setActive(true);

	logger.log(L"Screen Renderer Now Has Control of Active Game Window.");

	// Perform some OpenGL setup if needed
	if (!this->openGLInitialized) {

		logger.log(L"Initializing OpenGL...");

		this->openGLInitialized = true;

		// Enable and configure alpha blending
		glEnable(GL_BLEND);
		glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
		glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);

		// INITIALIZE SPRITE SHADER
		logger.log(L"Initializing sprite shader.");
		if (!spriteShader.initShader()) {
			logger.logError(L"Failed to initialize sprite shader.");
			exit(1);
		}

		// INITIALIZE THE SPRITES

		// General Sprites
		Stage->initSprite(spriteShader.getProgram());
		OpenCurtains->initSprite(spriteShader.getProgram());
		ClosedCurtainLeft->initSprite(spriteShader.getProgram());
		ClosedCurtainRight->initSprite(spriteShader.getProgram());
		SpotlightLeft->initSprite(spriteShader.getProgram());
		SpotlightRight->initSprite(spriteShader.getProgram());
		CurvySymbol->initSprite(spriteShader.getProgram());

		// Login
		TapLifeLinkPass->initSprite(spriteShader.getProgram());
		OR->initSprite(spriteShader.getProgram());
		BeginAsGuest->initSprite(spriteShader.getProgram());
		PosterA->initSprite(spriteShader.getProgram());
		PosterB->initSprite(spriteShader.getProgram());

		// UI Elements
		Frame->initSprite(spriteShader.getProgram());

		// Backgrounds
		SetMorning->initSprite(spriteShader.getProgram());
		SetAfternoon->initSprite(spriteShader.getProgram());

		// PreLogin Sprites
		LifeLinkIcon->initSprite(spriteShader.getProgram());

		// Playbills
		Act1->initSprite(spriteShader.getProgram());
		Act1Fin->initSprite(spriteShader.getProgram());
		Act2->initSprite(spriteShader.getProgram());
		Act2Fin->initSprite(spriteShader.getProgram());
		Fin->initSprite(spriteShader.getProgram());
		PreSelectAllBoxes->initSprite(spriteShader.getProgram());

		JacketArt->initSprite(spriteShader.getProgram());

		// INITIALIZE THE TEXTURES
		//TextureList::Inst()->PreloadTextures();

		// General Sprites
		Stage->setTextureID(TextureList::Inst()->GetTextureID("Textures/General/Stage.png"));
		OpenCurtains->setTextureID(TextureList::Inst()->GetTextureID("Textures/General/OpenCurtains.png"));
		ClosedCurtainLeft->setTextureID(TextureList::Inst()->GetTextureID("Textures/General/ClosedCurtainLeft.png"));
		ClosedCurtainRight->setTextureID(TextureList::Inst()->GetTextureID("Textures/General/ClosedCurtainRight.png"));
		SpotlightLeft->setTextureID(TextureList::Inst()->GetTextureID("Textures/General/Spotlights/Spotlight-Left.png"));
		SpotlightRight->setTextureID(TextureList::Inst()->GetTextureID("Textures/General/Spotlights/Spotlight-Right.png"));
		CurvySymbol->setTextureID(TextureList::Inst()->GetTextureID("Textures/General/CurvySymbol.png"));

		// Login
		TapLifeLinkPass->setTextureID(TextureList::Inst()->GetTextureID("Textures/Login/TapLifeLinkPass.png"));
		OR->setTextureID(TextureList::Inst()->GetTextureID("Textures/Login/OR.png"));
		BeginAsGuest->setTextureID(TextureList::Inst()->GetTextureID("Textures/Login/BeginAsGuest.png"));
		PosterA->setTextureID(TextureList::Inst()->GetTextureID("Textures/Login/PosterA.png"));
		PosterB->setTextureID(TextureList::Inst()->GetTextureID("Textures/Login/PosterB.png"));

		// UI Elements
		Frame->setTextureID(TextureList::Inst()->GetTextureID("Textures/General/Frame.png"));

		// Backgrounds
		SetMorning->setTextureID(TextureList::Inst()->GetTextureID("Textures/Backgrounds/Set-Morning.png"));
		SetAfternoon->setTextureID(TextureList::Inst()->GetTextureID("Textures/Backgrounds/Set-Afternoon.png"));

		// PreLogin Sprites
		LifeLinkIcon->setTextureID(TextureList::Inst()->GetTextureID("Textures/Login/LifeLink.png"));

		// Playbills
		Act1->setTextureID(TextureList::Inst()->GetTextureID("Textures/Playbills/Base Playbills/Playbill-Act1.png"));
		Act1Fin->setTextureID(TextureList::Inst()->GetTextureID("Textures/Playbills/Base Playbills/Playbill-Act1Fin.png"));
		Act2->setTextureID(TextureList::Inst()->GetTextureID("Textures/Playbills/Base Playbills/Playbill-Act2.png"));
		Act2Fin->setTextureID(TextureList::Inst()->GetTextureID("Textures/Playbills/Base Playbills/Playbill-Act2Fin.png"));
		Fin->setTextureID(TextureList::Inst()->GetTextureID("Textures/Playbills/Base Playbills/Playbill Fin.png"));
		PreSelectAllBoxes->setTextureID(TextureList::Inst()->GetTextureID("Textures/Playbills/Song Preselect Boxes/AllBoxes.png"));

		JacketArt->setTextureID(TextureList::Inst()->GetTextureID("Songs/+ERABY+E CONNEC+10N/jacket.png"));

		// SET THE TRANSFORMATIONS

		// General Sprites
		Stage->scale(2.f * aspect, 2.f, 1.f);
		OpenCurtains->scale(2.f * aspect, 2.f, 1.f);
		ClosedCurtainLeft->scale(2.f * aspect, 2.f, 1.f);
		ClosedCurtainRight->scale(2.f * aspect, 2.f, 1.f);
		SpotlightLeft->scale(2.f * aspect, 2.f, 1.f);
		SpotlightRight->scale(2.f * aspect, 2.f, 1.f);
		CurvySymbol->scale(.4f * aspect, .4f, 1.f);
		CurvySymbol->translate(0.f, -0.1f, 0.f);

		// Login
		TapLifeLinkPass->scale(.15f * 6.25f, .15f, 1.f);
		TapLifeLinkPass->translate(.75f, 0.08f, 0.f);
		OR->scale(0.2f * 1.5f, 0.2f, 1.f);
		OR->translate(-0.01f, 0.05f, 0.f);
		BeginAsGuest->scale(.4f * 2.25f, .4f, 1.f);
		BeginAsGuest->translate(-.75f, 0.f, 0.f);
		PosterA->scale(1.f, 1.3138f, 1.f);
		PosterA->translate(-0.5f, -0.2f, 0.f);
		PosterB->scale(1.f, 1.1368f, 1.f);
		PosterB->translate(0.5f, -0.2f, 0.f);

		// Backgrounds
		SetMorning->scale(2.f * aspect, 2.f, 1.f);
		SetAfternoon->scale(2.f * aspect, 2.f, 1.f);

		// PreLogin Sprites
		LifeLinkIcon->scale(.35f * aspect, .35f, 1.f);
		LifeLinkIcon->translate(.75f, -0.1f, 0.f);

		// Playbills
		Act1->scale(1.5f * aspect, 1.5f, 1.f);
		Act1->translate(0.f, -0.25f, 0.f);
		Act1Fin->scale(1.5f * aspect, 1.5f, 1.f);
		Act1Fin->translate(0.f, -0.25f, 0.f);
		Act2->scale(1.5f * aspect, 1.5f, 1.f);
		Act2->translate(0.f, -0.25f, 0.f);
		Act2Fin->scale(1.5f * aspect, 1.5f, 1.f);
		Act2Fin->translate(0.f, -0.25f, 0.f);
		Fin->scale(1.5f * aspect, 1.5f, 1.f);
		Fin->translate(0.f, -0.25f, 0.f);
		PreSelectAllBoxes->scale(1.5f * aspect, 1.5f, 1.f);
		PreSelectAllBoxes->translate(0.f, -0.25f, 0.f);

		// INITIALIZE TEXT SHADER
		logger.log(L"Initializing text shader.");
		if (!textShader.initShader()) {
			logger.logError(L"Failed to initialize text shader.");
			exit(1);
		}

		// INITIALIZE VIDEO SHADER
		if (!videoShader.initShader()) {
			logger.logError("Failed to initialize video shader.");
			exit(1);
		}

		// Initialize Videos
		if (!AttractTitleScreen->loadVideo("Videos/temp_AttractTitleScreen.mp4")) {
			logger.logError("Failed to load video: Videos/temp_AttractTitleScreen.mp4");
			exit(1);
		}

		// Link video sprite and shader
		AttractTitleScreen->initSprite(videoShader.getProgram());
		AttractTitleScreen->enableLooping(true);

		// Load the first frame
		AttractTitleScreen->scale(2.f * aspect, -2.f, 1.f);
		AttractTitleScreen->update(0);

		logger.log(L"OpenGL initialized.");
	}

	// Load Fonts
	TextureManager::TextureInfo HonyaJi = TextureList::Inst()->GetTextureInfo("Fonts/HonyaJi-Re.ttf");
	TextureManager::TextureInfo Stayola = TextureList::Inst()->GetTextureInfo("Fonts/Stayola-Regular.otf");

	// INITIALIZE TEXT

	// Login Details
	DisplayName = new OpenGLText(L"Display Name", *HonyaJi.font);
	UserTitle = new OpenGLText(L"User Title", *HonyaJi.font);
	Level = new OpenGLText(L"Level", *HonyaJi.font);
	PlayCount = new OpenGLText(L"Play Count", *HonyaJi.font);

	DisplayName->initSprite(textShader.getProgram());
	UserTitle->initSprite(textShader.getProgram());
	Level->initSprite(textShader.getProgram());
	PlayCount->initSprite(textShader.getProgram());

	DisplayName->scale(.75f);
	DisplayName->translate(-1200.f, 0.f, 0.f);

	UserTitle->scale(.4f);
	UserTitle->translate(-1200.f, -100.f, 0.f);

	// Clear color for the background
	glClearColor(0.f, 0.f, 0.f, 1.0f);
	glViewport(0, 0, 1920, 1080);

	thread startup;
	// Call the startup thread
	if (gameState.getGameState() == GameState::CurrentState::STARTUP) {
		startup = thread(execStartupChecks);
	}

	thread update;
	bool updateDownloadStarted = false;

	// Text Objects - FOR STARTUP
	OpenGLText* gameVersion = new OpenGLText(L"Game Version", *HonyaJi.font);
	OpenGLText* startupText = new OpenGLText(L"Startup", *HonyaJi.font);
	OpenGLText* networkText = new OpenGLText(L"Network", *HonyaJi.font);
	OpenGLText* updatesText = new OpenGLText(L"Updates", *HonyaJi.font);

	OpenGLText* colon1 = new OpenGLText(L"Colon 1", *HonyaJi.font);
	OpenGLText* colon2 = new OpenGLText(L"Colon 1", *HonyaJi.font);

	OpenGLText* networkStatusText = new OpenGLText(L"Network Status", *HonyaJi.font);
	OpenGLText* updatesStatusText = new OpenGLText(L"Updates Status", *HonyaJi.font);

	gameVersion->initSprite(textShader.getProgram());
	gameVersion->translate(-3100.f, 900.f, 0.f);
	gameVersion->scale(0.6f);

	startupText->initSprite(textShader.getProgram());
	startupText->translate(-2800.f, 750.f, 0.f);
	startupText->scale(0.6f);

	networkText->initSprite(textShader.getProgram());
	networkText->translate(-2100.f, 500.f, 0.f);
	networkText->scale(0.5f);

	updatesText->initSprite(textShader.getProgram());
	updatesText->translate(-2100.f, 350.f, 0.f);
	updatesText->scale(0.5f);

	colon1->initSprite(textShader.getProgram());
	colon1->translate(500.f, 500.f, 0.f);
	colon1->scale(0.5f);

	networkStatusText->initSprite(textShader.getProgram());
	networkStatusText->translate(700.f, 500.f, 0.f);
	networkStatusText->scale(0.5f);

	colon2->initSprite(textShader.getProgram());
	colon2->translate(500.f, 350.f, 0.f);
	colon2->scale(0.5f);

	updatesStatusText->initSprite(textShader.getProgram());
	updatesStatusText->translate(700.f, 350.f, 0.f);
	updatesStatusText->scale(0.5f);

	// Text Objects - For Test Menu
	OpenGLText* testMenuTitle = new OpenGLText(L"Test Menu Title", *HonyaJi.font);
	OpenGLText* testMenuText1 = new OpenGLText(L"Test Menu Text 1", *HonyaJi.font);
	OpenGLText* testMenuText2 = new OpenGLText(L"Test Menu Text 2", *HonyaJi.font);
	OpenGLText* testMenuText3 = new OpenGLText(L"Test Menu Text 3", *HonyaJi.font);
	OpenGLText* testMenuText4 = new OpenGLText(L"Test Menu Text 4", *HonyaJi.font);
	OpenGLText* testMenuText5 = new OpenGLText(L"Test Menu Text 5", *HonyaJi.font);
	OpenGLText* testMenuText6 = new OpenGLText(L"Test Menu Text 6", *HonyaJi.font);
	OpenGLText* testMenuText7 = new OpenGLText(L"Test Menu Text 7", *HonyaJi.font);
	OpenGLText* testMenuText8 = new OpenGLText(L"Test Menu Text 8", *HonyaJi.font);

	testMenuTitle->initSprite(textShader.getProgram());
	testMenuText1->initSprite(textShader.getProgram());
	testMenuText2->initSprite(textShader.getProgram());
	testMenuText3->initSprite(textShader.getProgram());
	testMenuText4->initSprite(textShader.getProgram());
	testMenuText5->initSprite(textShader.getProgram());
	testMenuText6->initSprite(textShader.getProgram());
	testMenuText7->initSprite(textShader.getProgram());
	testMenuText8->initSprite(textShader.getProgram());

	// Timer used for countdowns
	bool timerRunning = false;

	// Forward declaration of this thread for use in login
	sf::Thread loginThread(&checkForProfile);

	// Create time variables
	typedef std::chrono::high_resolution_clock Time;
	typedef std::chrono::milliseconds ms;
	typedef std::chrono::duration<float> fsec;
	auto startTime = Time::now();

	// The rendering loop
	/*
		The loop will break either if the ESC key is pressed switching
		the game state to SHUTDOWN or if the '.' key is pressed triggering
		the service flag to be set to true
	*/
	while (gameWindow->isOpen() && !(gameState.getGameState() == GameState::CurrentState::SHUTDOWN) && !gameState.checkService()) {

		// Set the current time
		auto currentTime = Time::now();
		fsec fs = currentTime - startTime;
		ms currentOffset = std::chrono::duration_cast<ms>(fs);
		// Use currentOffset.count() to get millisecond value

		// Check if switching to the GAME game state
		if (gameState.getGameState() == GameState::CurrentState::GAME) {
			// Clear the buffers
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			// Set the screen to not active
			gameWindow->setActive(false);

			// Launch the game renderer in a new thread and wait for it to complete
			sf::Thread thread(&rThread, gameWindow);
			thread.launch();
			thread.wait();

			// Re-activate the game window in this thread
			gameWindow->setActive(true);

			// Transition to results since a song has now completed while making sure the shutdown key wasn't pressed to trigger the end of the thread
			if (gameState.getGameState() != GameState::CurrentState::SHUTDOWN) {
				gameState.setGameState(GameState::CurrentState::RESULTS);
			}
		}

		// Launch update download thread if needed
		if (gameState.getGameState() == GameState::CurrentState::UPDATES && updateDownloadStarted == false) {
			updateDownloadStarted = true;
			update = thread(execDownloadUpdate);
		}

		// Clear the buffers
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Use the sprite shader first
		glUseProgram(spriteShader.getProgram());

		// DRAW BACKGROUNDS / STAGE (MAIN SET)
		{
			if (gameState.isInServiceGameState()) {
				// Don't render anything background related when in a service state
			}
			else if (gameState.getGameState() == GameState::CurrentState::TITLE_SCREEN) {
				// Swap to the video shader program
				glUseProgram(videoShader.getProgram());

				// Update the current frame
				AttractTitleScreen->update((double)fs.count());

				// Render that frame
				AttractTitleScreen->render(PROJECTION::ORTHOGRAPHIC);
			}
			else {
				// Render Background
				if (gameState.getGameState() == GameState::CurrentState::PRELOGIN) {
					SetMorning->render(PROJECTION::ORTHOGRAPHIC);
				}
				else if (gameState.getGameState() == GameState::CurrentState::LOGIN_DETAILS) {
					SetMorning->render(PROJECTION::ORTHOGRAPHIC);
				}
				else if (gameState.getGameState() == GameState::CurrentState::SONG_SELECT) {
					SetAfternoon->render(PROJECTION::ORTHOGRAPHIC);
				}

				// Render Set
				Stage->render(PROJECTION::ORTHOGRAPHIC);
				OpenCurtains->render(PROJECTION::ORTHOGRAPHIC);
				SpotlightLeft->render(PROJECTION::ORTHOGRAPHIC);
				SpotlightRight->render(PROJECTION::ORTHOGRAPHIC);
			}
		}

		// Set the sprite shader back in case the video shader was being used
		glUseProgram(spriteShader.getProgram());

		// Start Timers for scenes
		{
			if (gameState.getGameState() == GameState::CurrentState::PRELOGIN) {
				if (!timerRunning) {
					// Timer hasn't been started yet
					// TODO: START THE 30 SECOND TIMER

					// Set timer to running
					timerRunning = true;
				}
				else {
					// Timer is started
					// TODO: CHECK IF 0 AND GO BACK TO TITLE SCREEN
				}
			}
		}

		// Check for card tapped on Title or Prelogin states
		if (gameState.getGameState() == GameState::CurrentState::TITLE_SCREEN || gameState.getGameState() == GameState::CurrentState::PRELOGIN) {
			if (RFIDCardReader::getCardReader()->getLastCardData() != "") {
				// If card tapped on the title screen, move to the prelogin screen
				if (gameState.getGameState() == GameState::CurrentState::TITLE_SCREEN) {
					gameState.setGameState(GameState::CurrentState::PRELOGIN);
				}

				// Check the state of the login
				switch (loginComplete) {
					case -1:
						// Play sound effect for card tapped
						soundEffects.playSoundEffect(SoundEffects::Effects::FX_CardTap);

						// Launch a thread to attempt getting the profile data
						loginThread.launch();

						// Set variable so we know it is checking
						loginComplete = 0;
						break;
					case 0:
						// Attempting to login
						break;
					case 1:
						if (userData.isNewUser()) {
							// User has a profile already
							gameState.setGameState(GameState::CurrentState::LOGIN_DETAILS);
						}
						else {
							// User doesn't have a profile yet, go to profile creation
							gameState.setGameState(GameState::CurrentState::CREATE_PROFILE);
						}
						controllerInput.reset();
						timerRunning = false;
						break;
					case 2:
						// Can't login as we were unable to connect or some other error occurred
						soundEffects.playSoundEffect(SoundEffects::Effects::FX_Error);
						loginComplete = -1;
						break;
				}
			}
		}

		// DRAW ALL OTHER GRAPHICS
		{
			if (gameState.getGameState() == GameState::CurrentState::PRELOGIN) {
				Frame->reset();
				Frame->scale(.6f * aspect, .6f, 1.f);
				Frame->translate(-.75f, 0.f, 0.f);
				Frame->render(PROJECTION::ORTHOGRAPHIC);

				Frame->translate(1.5f, 0.f, 0.f);
				Frame->render(PROJECTION::ORTHOGRAPHIC);

				BeginAsGuest->render(PROJECTION::ORTHOGRAPHIC);

				TapLifeLinkPass->render(PROJECTION::ORTHOGRAPHIC);
				LifeLinkIcon->render(PROJECTION::ORTHOGRAPHIC);

				OR->render(PROJECTION::ORTHOGRAPHIC);
				CurvySymbol->render(PROJECTION::ORTHOGRAPHIC);
			}
			else if (gameState.getGameState() == GameState::CurrentState::LOGIN_DETAILS) {
				PosterA->render(PROJECTION::ORTHOGRAPHIC);
				PosterB->render(PROJECTION::ORTHOGRAPHIC);
			}
			else if (gameState.getGameState() == GameState::CurrentState::SONG_SELECT) {
				if (gameState.results.size() == 0) {
					// Song 1 - Act 1
					Act1->render(PROJECTION::ORTHOGRAPHIC);
				}
				else if (gameState.results.size() == 1) {
					// Song 2 - Act 2
					Act2->render(PROJECTION::ORTHOGRAPHIC);
				}
				else {
					// Error or EX track
					// NOT IMPLEMENTED
				}

				PreSelectAllBoxes->render(PROJECTION::ORTHOGRAPHIC);

				JacketArt->render(PROJECTION::ORTHOGRAPHIC);
			}
		}

		// Switch over to the text shader
		glUseProgram(textShader.getProgram());

		// DRAW ALL TEXT
		{
			if (gameState.getGameState() == GameState::CurrentState::STARTUP) {
				int netCheck = 0;
				int updateCheck = 0;

				gameVersion->render(PROJECTION::ORTHOGRAPHIC, "SNA: " + network.getLocalVersion());
				startupText->render(PROJECTION::ORTHOGRAPHIC, "SONATARIA STARTUP...");
				networkText->render(PROJECTION::ORTHOGRAPHIC, "NETWORK");
				colon1->render(PROJECTION::ORTHOGRAPHIC, ":");
				updatesText->render(PROJECTION::ORTHOGRAPHIC, "UPDATE CHECK");
				colon2->render(PROJECTION::ORTHOGRAPHIC, ":");

				switch (networkStatus) {
					default:
					case -1:
						networkStatusText->render(PROJECTION::ORTHOGRAPHIC, "WAITING");
						break;
					case 0:
						networkStatusText->render(PROJECTION::ORTHOGRAPHIC, "CHECKING", ALIGNMENT::LEFT, 1.f, 1.f, 0.f);
						break;
					case 1:
						networkStatusText->render(PROJECTION::ORTHOGRAPHIC, "ONLINE", ALIGNMENT::LEFT, 0.f, 1.f, 0.f);
						break;
					case 2:
						networkStatusText->render(PROJECTION::ORTHOGRAPHIC, "OFFLINE", ALIGNMENT::LEFT, 1.f, 0.f, 0.f);
						break;
					case 3:
						networkStatusText->render(PROJECTION::ORTHOGRAPHIC, "MAINTENENCE", ALIGNMENT::LEFT, 1.f, 1.f, 0.f);
						break;
				}

				switch (updateCheckStatus) {
					case -2:
						updatesStatusText->render(PROJECTION::ORTHOGRAPHIC, "WAITING");
						break;
					default:
					case -1:
						updatesStatusText->render(PROJECTION::ORTHOGRAPHIC, "SKIP", ALIGNMENT::LEFT, 1.f, 1.f, 0.f);
						break;
					case 0:
						updatesStatusText->render(PROJECTION::ORTHOGRAPHIC, "CHECKING", ALIGNMENT::LEFT, 1.f, 1.f, 0.f);
						break;
					case 1:
						updatesStatusText->render(PROJECTION::ORTHOGRAPHIC, "AVAILABLE", ALIGNMENT::LEFT, 0.f, 1.f, 0.f);
						break;
					case 2:
						updatesStatusText->render(PROJECTION::ORTHOGRAPHIC, "NONE", ALIGNMENT::LEFT, 1.f, 0.f, 0.f);
						break;
				}
			}
			else if (gameState.getGameState() == GameState::CurrentState::TEST_MENU_MAIN) {
				testMenuTitle->reset();
				testMenuTitle->translate(0.f, 800.f, 0.f);
				testMenuTitle->scale(0.5f);
				testMenuTitle->render(PROJECTION::ORTHOGRAPHIC, L"MAIN MENU", ALIGNMENT::CENTERED);

				testMenuText1->reset();
				testMenuText1->translate(0.f, 450.f, 0.f);
				testMenuText1->scale(0.5f);

				testMenuText2->reset();
				testMenuText2->translate(0.f, 350.f, 0.f);
				testMenuText2->scale(0.5f);

				testMenuText3->reset();
				testMenuText3->translate(0.f, 250.f, 0.f);
				testMenuText3->scale(0.5f);

				testMenuText4->reset();
				testMenuText4->translate(0.f, 150.f, 0.f);
				testMenuText4->scale(0.5f);

				testMenuText5->reset();
				testMenuText5->translate(0.f, -450.f, 0.f);
				testMenuText5->scale(0.5f);

				testMenuText6->reset();
				testMenuText6->translate(0.f, -850.f, 0.f);
				testMenuText6->scale(0.35f);

				testMenuText7->reset();
				testMenuText7->translate(0.f, -925.f, 0.f);
				testMenuText7->scale(0.35f);

				testMenuText8->reset();
				testMenuText8->translate(0.f, -1000.f, 0.f);
				testMenuText8->scale(0.35f);

				switch (testMenuPos) {
					default:
					case 0:
						testMenuText1->render(PROJECTION::ORTHOGRAPHIC, L"I/O CHECK", ALIGNMENT::CENTERED, 1.f, 0.f, 0.f);
						testMenuText2->render(PROJECTION::ORTHOGRAPHIC, L"SOUND OPTIONS", ALIGNMENT::CENTERED, 1.f, 1.f, 1.f);
						testMenuText3->render(PROJECTION::ORTHOGRAPHIC, L"NETWORKING", ALIGNMENT::CENTERED, 1.f, 1.f, 1.f);
						testMenuText4->render(PROJECTION::ORTHOGRAPHIC, L"SYSTEM INFORMATION", ALIGNMENT::CENTERED, 1.f, 1.f, 1.f);
						testMenuText5->render(PROJECTION::ORTHOGRAPHIC, L"GAME MODE", ALIGNMENT::CENTERED, 1.f, 1.f, 1.f);
						break;
					case 1:
						testMenuText1->render(PROJECTION::ORTHOGRAPHIC, L"I/O CHECK", ALIGNMENT::CENTERED, 1.f, 1.f, 1.f);
						testMenuText2->render(PROJECTION::ORTHOGRAPHIC, L"SOUND OPTIONS", ALIGNMENT::CENTERED, 1.f, 0.f, 0.f);
						testMenuText3->render(PROJECTION::ORTHOGRAPHIC, L"NETWORKING", ALIGNMENT::CENTERED, 1.f, 1.f, 1.f);
						testMenuText4->render(PROJECTION::ORTHOGRAPHIC, L"SYSTEM INFORMATION", ALIGNMENT::CENTERED, 1.f, 1.f, 1.f);
						testMenuText5->render(PROJECTION::ORTHOGRAPHIC, L"GAME MODE", ALIGNMENT::CENTERED, 1.f, 1.f, 1.f);
						break;
					case 2:
						testMenuText1->render(PROJECTION::ORTHOGRAPHIC, L"I/O CHECK", ALIGNMENT::CENTERED, 1.f, 1.f, 1.f);
						testMenuText2->render(PROJECTION::ORTHOGRAPHIC, L"SOUND OPTIONS", ALIGNMENT::CENTERED, 1.f, 1.f, 1.f);
						testMenuText3->render(PROJECTION::ORTHOGRAPHIC, L"NETWORKING", ALIGNMENT::CENTERED, 1.f, 0.f, 0.f);
						testMenuText4->render(PROJECTION::ORTHOGRAPHIC, L"SYSTEM INFORMATION", ALIGNMENT::CENTERED, 1.f, 1.f, 1.f);
						testMenuText5->render(PROJECTION::ORTHOGRAPHIC, L"GAME MODE", ALIGNMENT::CENTERED, 1.f, 1.f, 1.f);
						break;
					case 3:
						testMenuText1->render(PROJECTION::ORTHOGRAPHIC, L"I/O CHECK", ALIGNMENT::CENTERED, 1.f, 1.f, 1.f);
						testMenuText2->render(PROJECTION::ORTHOGRAPHIC, L"SOUND OPTIONS", ALIGNMENT::CENTERED, 1.f, 1.f, 1.f);
						testMenuText3->render(PROJECTION::ORTHOGRAPHIC, L"NETWORKING", ALIGNMENT::CENTERED, 1.f, 1.f, 1.f);
						testMenuText4->render(PROJECTION::ORTHOGRAPHIC, L"SYSTEM INFORMATION", ALIGNMENT::CENTERED, 1.f, 0.f, 0.f);
						testMenuText5->render(PROJECTION::ORTHOGRAPHIC, L"GAME MODE", ALIGNMENT::CENTERED, 1.f, 1.f, 1.f);
						break;
					case 4:
						testMenuText1->render(PROJECTION::ORTHOGRAPHIC, L"I/O CHECK", ALIGNMENT::CENTERED, 1.f, 1.f, 1.f);
						testMenuText2->render(PROJECTION::ORTHOGRAPHIC, L"SOUND OPTIONS", ALIGNMENT::CENTERED, 1.f, 1.f, 1.f);
						testMenuText3->render(PROJECTION::ORTHOGRAPHIC, L"NETWORKING", ALIGNMENT::CENTERED, 1.f, 1.f, 1.f);
						testMenuText4->render(PROJECTION::ORTHOGRAPHIC, L"SYSTEM INFORMATION", ALIGNMENT::CENTERED, 1.f, 1.f, 1.f);
						testMenuText5->render(PROJECTION::ORTHOGRAPHIC, L"GAME MODE", ALIGNMENT::CENTERED, 1.f, 0.f, 0.f);
						break;
					}

					testMenuText6->render(PROJECTION::ORTHOGRAPHIC, L"BT-START [T] | SELECT", ALIGNMENT::CENTERED);
					testMenuText7->render(PROJECTION::ORTHOGRAPHIC, L"BT-2 [C] | DOWN", ALIGNMENT::CENTERED);
					testMenuText8->render(PROJECTION::ORTHOGRAPHIC, L"BT-4 [M] | UP", ALIGNMENT::CENTERED);
			}
			else if (gameState.getGameState() == GameState::CurrentState::TEST_MENU_SYSINFO) {
				testMenuTitle->reset();
				testMenuTitle->translate(0.f, 800.f, 0.f);
				testMenuTitle->scale(0.5f);
				testMenuTitle->render(PROJECTION::ORTHOGRAPHIC, L"SYSTEM INFORMATION", ALIGNMENT::CENTERED);

				testMenuText1->reset();
				testMenuText1->translate(-1200.f, 350.f, 0.f);
				testMenuText1->scale(0.5f);
				testMenuText1->render(PROJECTION::ORTHOGRAPHIC, L"VERSION ID", ALIGNMENT::CENTERED);

				testMenuText2->reset();
				testMenuText2->translate(400.f, 350.f, 0.f);
				testMenuText2->scale(0.5f);
				testMenuText2->render(PROJECTION::ORTHOGRAPHIC, "SNA:" + network.getLocalVersion(), ALIGNMENT::LEFT);

				testMenuText3->reset();
				testMenuText3->translate(0.f, -650.f, 0.f);
				testMenuText3->scale(0.5f);
				testMenuText3->render(PROJECTION::ORTHOGRAPHIC, L"EXIT", ALIGNMENT::CENTERED, 1.f, 0.f, 0.f);

				testMenuText4->reset();
				testMenuText4->translate(0.f, -925.f, 0.f);
				testMenuText4->scale(0.35f);
				testMenuText4->render(PROJECTION::ORTHOGRAPHIC, L"BT-START [T] | SELECT", ALIGNMENT::CENTERED);
			}
			else if (gameState.getGameState() == GameState::CurrentState::TEST_MENU_NETWORKING) {
				testMenuTitle->reset();
				testMenuTitle->translate(0.f, 800.f, 0.f);
				testMenuTitle->scale(0.5f);
				testMenuTitle->render(PROJECTION::ORTHOGRAPHIC, L"NETWORKING", ALIGNMENT::CENTERED);

				testMenuText1->reset();
				testMenuText1->translate(-1200.f, 350.f, 0.f);
				testMenuText1->scale(0.5f);
				testMenuText1->render(PROJECTION::ORTHOGRAPHIC, L"NETWORK STATUS", ALIGNMENT::CENTERED);

				testMenuText2->reset();
				testMenuText2->translate(700.f, 350.f, 0.f);
				testMenuText2->scale(0.5f);
				switch (gameState.getOnlineState()) {
					case GameState::OnlineState::MAINTENENCE:
						testMenuText2->render(PROJECTION::ORTHOGRAPHIC, L"MAINTENENCE", ALIGNMENT::LEFT, 1.f, 1.f, 0.f);
						break;
					case GameState::OnlineState::OFFLINE:
						testMenuText2->render(PROJECTION::ORTHOGRAPHIC, L"OFFLINE", ALIGNMENT::LEFT, 1.f, 0.f, 0.f);
						break;
					case GameState::OnlineState::ONLINE:
						testMenuText2->render(PROJECTION::ORTHOGRAPHIC, L"ONLINE", ALIGNMENT::LEFT, 0.f, 1.f, 0.f);
						break;
				}

				testMenuText3->reset();
				testMenuText3->translate(0.f, -575.f, 0.f);
				testMenuText3->scale(0.5f);

				testMenuText4->reset();
				testMenuText4->translate(0.f, -650.f, 0.f);
				testMenuText4->scale(0.5f);

				switch (this->testMenuNetworkingPos) {
					case 0:
						if (this->isNetworkChecking == true) {
							testMenuText3->render(PROJECTION::ORTHOGRAPHIC, L"CHECKING...", ALIGNMENT::CENTERED, 1.f, 1.f, 0.f);
							testMenuText4->render(PROJECTION::ORTHOGRAPHIC, L"EXIT", ALIGNMENT::CENTERED);
						}
						else {
							testMenuText3->render(PROJECTION::ORTHOGRAPHIC, L"TEST CONNECTION", ALIGNMENT::CENTERED, 1.f, 0.f, 0.f);
							testMenuText4->render(PROJECTION::ORTHOGRAPHIC, L"EXIT", ALIGNMENT::CENTERED);
						}
						break;
					case 1:
						testMenuText3->render(PROJECTION::ORTHOGRAPHIC, L"TEST CONNECTION", ALIGNMENT::CENTERED);
						testMenuText4->render(PROJECTION::ORTHOGRAPHIC, L"EXIT", ALIGNMENT::CENTERED, 1.f, 0.f, 0.f);
						break;
				}

				testMenuText5->reset();
				testMenuText5->translate(0.f, -850.f, 0.f);
				testMenuText5->scale(0.35f);
				testMenuText5->render(PROJECTION::ORTHOGRAPHIC, L"BT-START [T] | SELECT", ALIGNMENT::CENTERED);

				testMenuText6->reset();
				testMenuText6->translate(0.f, -925.f, 0.f);
				testMenuText6->scale(0.35f);
				testMenuText6->render(PROJECTION::ORTHOGRAPHIC, L"BT-2 [C] | DOWN", ALIGNMENT::CENTERED);

				testMenuText7->reset();
				testMenuText7->translate(0.f, -1000.f, 0.f);
				testMenuText7->scale(0.35f);
				testMenuText7->render(PROJECTION::ORTHOGRAPHIC, L"BT-4 [M] | UP", ALIGNMENT::CENTERED);
			}
			else if (gameState.getGameState() == GameState::CurrentState::TEST_MENU_IOCHECK) {
				testMenuTitle->reset();
				testMenuTitle->translate(0.f, 800.f, 0.f);
				testMenuTitle->scale(0.5f);
				testMenuTitle->render(PROJECTION::ORTHOGRAPHIC, L"I/O CHECK", ALIGNMENT::CENTERED);

				testMenuText2->reset();
				testMenuText2->translate(0.f, 400.f, 0.f);
				testMenuText2->scale(0.5f);

				testMenuText3->reset();
				testMenuText3->translate(0.f, -650.f, 0.f);
				testMenuText3->scale(0.5f);

				switch (this->testMenuIOChkPos) {
					case 0:
						testMenuText2->render(PROJECTION::ORTHOGRAPHIC, L"INPUT CHECK", ALIGNMENT::CENTERED, 1.f, 0.f, 0.f);
						testMenuText3->render(PROJECTION::ORTHOGRAPHIC, L"EXIT", ALIGNMENT::CENTERED);
						break;
					case 1:
						testMenuText2->render(PROJECTION::ORTHOGRAPHIC, L"INPUT CHECK", ALIGNMENT::CENTERED);
						testMenuText3->render(PROJECTION::ORTHOGRAPHIC, L"EXIT", ALIGNMENT::CENTERED, 1.f, 0.f, 0.f);
						break;
				}

				testMenuText4->reset();
				testMenuText4->translate(0.f, -850.f, 0.f);
				testMenuText4->scale(0.35f);

				testMenuText5->reset();
				testMenuText5->translate(0.f, -925.f, 0.f);
				testMenuText5->scale(0.35f);

				testMenuText6->reset();
				testMenuText6->translate(0.f, -1000.f, 0.f);
				testMenuText6->scale(0.35f);

				testMenuText4->render(PROJECTION::ORTHOGRAPHIC, L"BT-START [T] | SELECT", ALIGNMENT::CENTERED);
				testMenuText5->render(PROJECTION::ORTHOGRAPHIC, L"BT-2 [C] | DOWN", ALIGNMENT::CENTERED);
				testMenuText6->render(PROJECTION::ORTHOGRAPHIC, L"BT-4 [M] | UP", ALIGNMENT::CENTERED);
			}
			else if (gameState.getGameState() == GameState::CurrentState::TEST_MENU_INPUTCHECK) {
				testMenuTitle->reset();
				testMenuTitle->translate(0.f, 800.f, 0.f);
				testMenuTitle->scale(0.5f);
				testMenuTitle->render(PROJECTION::ORTHOGRAPHIC, L"I/O CHECK", ALIGNMENT::CENTERED);

				testMenuText1->reset();
				testMenuText1->translate(0.f, 725.f, 0.f);
				testMenuText1->scale(0.5f);
				testMenuText1->render(PROJECTION::ORTHOGRAPHIC, L"INPUT CHECK", ALIGNMENT::CENTERED);

				testMenuText1->reset();
				testMenuText1->translate(-1200.f, 350.f, 0.f);
				testMenuText1->scale(0.5f);
				testMenuText1->render(PROJECTION::ORTHOGRAPHIC, L"1 BUTTON", ALIGNMENT::LEFT);

				testMenuText1->reset();
				testMenuText1->translate(-1200.f, 250.f, 0.f);
				testMenuText1->scale(0.5f);
				testMenuText1->render(PROJECTION::ORTHOGRAPHIC, L"2 BUTTON", ALIGNMENT::LEFT);

				testMenuText1->reset();
				testMenuText1->translate(-1200.f, 150.f, 0.f);
				testMenuText1->scale(0.5f);
				testMenuText1->render(PROJECTION::ORTHOGRAPHIC, L"3 BUTTON", ALIGNMENT::LEFT);

				testMenuText1->reset();
				testMenuText1->translate(-1200.f, 50.f, 0.f);
				testMenuText1->scale(0.5f);
				testMenuText1->render(PROJECTION::ORTHOGRAPHIC, L"4 BUTTON", ALIGNMENT::LEFT);

				testMenuText1->reset();
				testMenuText1->translate(-1200.f, -50.f, 0.f);
				testMenuText1->scale(0.5f);
				testMenuText1->render(PROJECTION::ORTHOGRAPHIC, L"5 BUTTON", ALIGNMENT::LEFT);

				testMenuText1->reset();
				testMenuText1->translate(-1200.f, -150.f, 0.f);
				testMenuText1->scale(0.5f);
				testMenuText1->render(PROJECTION::ORTHOGRAPHIC, L"WHEEL", ALIGNMENT::LEFT);

				testMenuText1->reset();
				testMenuText1->translate(600.f, 350.f, 0.f);
				testMenuText1->scale(0.5f);

				if (controllerInput.getKeyboardState().getKeyState(1)) {
					testMenuText1->render(PROJECTION::ORTHOGRAPHIC, L"ON", ALIGNMENT::LEFT, 0.f, 1.f, 0.f);
				}
				else {
					testMenuText1->render(PROJECTION::ORTHOGRAPHIC, L"OFF", ALIGNMENT::LEFT, 1.f, 0.f, 0.f);
				}

				testMenuText1->reset();
				testMenuText1->translate(600.f, 250.f, 0.f);
				testMenuText1->scale(0.5f);

				if (controllerInput.getKeyboardState().getKeyState(2)) {
					testMenuText1->render(PROJECTION::ORTHOGRAPHIC, L"ON", ALIGNMENT::LEFT, 0.f, 1.f, 0.f);
				}
				else {
					testMenuText1->render(PROJECTION::ORTHOGRAPHIC, L"OFF", ALIGNMENT::LEFT, 1.f, 0.f, 0.f);
				}

				testMenuText1->reset();
				testMenuText1->translate(600.f, 150.f, 0.f);
				testMenuText1->scale(0.5f);

				if (controllerInput.getKeyboardState().getKeyState(3)) {
					testMenuText1->render(PROJECTION::ORTHOGRAPHIC, L"ON", ALIGNMENT::LEFT, 0.f, 1.f, 0.f);
				}
				else {
					testMenuText1->render(PROJECTION::ORTHOGRAPHIC, L"OFF", ALIGNMENT::LEFT, 1.f, 0.f, 0.f);
				}

				testMenuText1->reset();
				testMenuText1->translate(600.f, 50.f, 0.f);
				testMenuText1->scale(0.5f);

				if (controllerInput.getKeyboardState().getKeyState(4)) {
					testMenuText1->render(PROJECTION::ORTHOGRAPHIC, L"ON", ALIGNMENT::LEFT, 0.f, 1.f, 0.f);
				}
				else {
					testMenuText1->render(PROJECTION::ORTHOGRAPHIC, L"OFF", ALIGNMENT::LEFT, 1.f, 0.f, 0.f);
				}

				testMenuText1->reset();
				testMenuText1->translate(600.f, -50.f, 0.f);
				testMenuText1->scale(0.5f);

				if (controllerInput.getKeyboardState().getKeyState(5)) {
					testMenuText1->render(PROJECTION::ORTHOGRAPHIC, L"ON", ALIGNMENT::LEFT, 0.f, 1.f, 0.f);
				}
				else {
					testMenuText1->render(PROJECTION::ORTHOGRAPHIC, L"OFF", ALIGNMENT::LEFT, 1.f, 0.f, 0.f);
				}

				testMenuText1->reset();
				testMenuText1->translate(600.f, -150.f, 0.f);
				testMenuText1->scale(0.5f);
				testMenuText1->render(PROJECTION::ORTHOGRAPHIC, to_string(controllerInput.getWheelPos()), ALIGNMENT::LEFT);

				testMenuText1->reset();
				testMenuText1->translate(0.f, -650.f, 0.f);
				testMenuText1->scale(0.5f);
				testMenuText1->render(PROJECTION::ORTHOGRAPHIC, L"EXIT", ALIGNMENT::CENTERED, 1.f, 0.f, 0.f);

				testMenuText1->reset();
				testMenuText1->translate(0.f, -850.f, 0.f);
				testMenuText1->scale(0.35f);
				testMenuText1->render(PROJECTION::ORTHOGRAPHIC, L"BT-START [T] | SELECT", ALIGNMENT::CENTERED);
			}
			else if (gameState.getGameState() == GameState::CurrentState::TEST_MENU_SOUNDOPTIONS) {
				testMenuTitle->reset();
				testMenuTitle->translate(0.f, 800.f, 0.f);
				testMenuTitle->scale(0.5f);
				testMenuTitle->render(PROJECTION::ORTHOGRAPHIC, L"SOUND OPTIONS", ALIGNMENT::CENTERED);

				testMenuText1->reset();
				testMenuText1->translate(-1200.f, 350.f, 0.f);
				testMenuText1->scale(0.5f);
				

				testMenuText2->reset();
				testMenuText2->translate(600.f, 350.f, 0.f);
				testMenuText2->scale(0.5f);
				string sysVol = to_string(round(windowsAudio.GetSystemVolume(WindowsAudio::VolumeUnit::Scalar) * 100.f));
				sysVol.erase(sysVol.find_last_not_of('0') + 1, std::string::npos);
				sysVol.erase(sysVol.find_last_not_of('.') + 1, std::string::npos);
				testMenuText2->render(PROJECTION::ORTHOGRAPHIC, sysVol, ALIGNMENT::LEFT);

				testMenuText3->reset();
				testMenuText3->translate(0.f, -650.f, 0.f);
				testMenuText3->scale(0.5f);
				

				switch (this->testMenuSoundOptionsPos) {
					case 0:
						if (this->testMenuSoundOptionsSelected) {
							testMenuText1->render(PROJECTION::ORTHOGRAPHIC, L"System Volume", ALIGNMENT::LEFT, 0.f, 1.f, 0.f);
						}
						else {
							testMenuText1->render(PROJECTION::ORTHOGRAPHIC, L"System Volume", ALIGNMENT::LEFT, 1.f, 0.f, 0.f);
						}
						testMenuText3->render(PROJECTION::ORTHOGRAPHIC, L"EXIT", ALIGNMENT::CENTERED);
						break;
					case 1:
						testMenuText1->render(PROJECTION::ORTHOGRAPHIC, L"System Volume", ALIGNMENT::LEFT);
						testMenuText3->render(PROJECTION::ORTHOGRAPHIC, L"EXIT", ALIGNMENT::CENTERED, 1.f, 0.f, 0.f);
				}

				testMenuText1->reset();
				testMenuText1->translate(0.f, -850.f, 0.f);
				testMenuText1->scale(0.35f);
				testMenuText1->render(PROJECTION::ORTHOGRAPHIC, L"BT-START [T] | SELECT", ALIGNMENT::CENTERED);

				testMenuText1->reset();
				testMenuText1->translate(0.f, -925.f, 0.f);
				testMenuText1->scale(0.35f);
				testMenuText1->render(PROJECTION::ORTHOGRAPHIC, L"BT-2 [C] | DOWN", ALIGNMENT::CENTERED);

				testMenuText1->reset();
				testMenuText1->translate(0.f, -1000.f, 0.f);
				testMenuText1->scale(0.35f);
				testMenuText1->render(PROJECTION::ORTHOGRAPHIC, L"BT-4 [M] | UP", ALIGNMENT::CENTERED);
			}
			else if (gameState.getGameState() == GameState::CurrentState::UPDATES) {
				testMenuTitle->reset();
				testMenuTitle->translate(0.f, 800.f, 0.f);
				testMenuTitle->scale(0.5f);
				testMenuTitle->render(PROJECTION::ORTHOGRAPHIC, L"SYSTEM UPDATE", ALIGNMENT::CENTERED);

				testMenuText1->reset();
				testMenuText1->translate(-1200.f, 350.f, 0.f);
				testMenuText1->scale(0.5f);
				testMenuText1->render(PROJECTION::ORTHOGRAPHIC, network.getLocalVersion(), ALIGNMENT::CENTERED);

				testMenuText1->reset();
				testMenuText1->translate(0.f, 350.f, 0.f);
				testMenuText1->scale(0.5f);
				testMenuText1->render(PROJECTION::ORTHOGRAPHIC, L"->", ALIGNMENT::CENTERED);

				testMenuText1->reset();
				testMenuText1->translate(400.f, 350.f, 0.f);
				testMenuText1->scale(0.5f);
				testMenuText1->render(PROJECTION::ORTHOGRAPHIC, newVersion, ALIGNMENT::LEFT);

				testMenuText1->reset();
				testMenuText1->translate(0.f, -650.f, 0.f);
				testMenuText1->scale(0.5f);

				switch (updateDownloadStatus) {
					default:
					case -1:
						testMenuText1->render(PROJECTION::ORTHOGRAPHIC, L"Preparing...", ALIGNMENT::CENTERED);
						break;
					case 0:
						testMenuText1->render(PROJECTION::ORTHOGRAPHIC, L"Downloading...", ALIGNMENT::CENTERED);
						break;
					case 1:
testMenuText1->render(PROJECTION::ORTHOGRAPHIC, L"Extracting...", ALIGNMENT::CENTERED);
break;
					case 2:
						testMenuText1->render(PROJECTION::ORTHOGRAPHIC, L"Modifying Startup...", ALIGNMENT::CENTERED);
						break;
					case 3:
						testMenuText1->render(PROJECTION::ORTHOGRAPHIC, L"Restarting...", ALIGNMENT::CENTERED);
						break;
				}

				testMenuText1->reset();
				testMenuText1->translate(0.f, -850.f, 0.f);
				testMenuText1->scale(0.35f);
				testMenuText1->render(PROJECTION::ORTHOGRAPHIC, L"DO NOT POWER OFF OR DISCONNECT FROM NETWORK!", ALIGNMENT::CENTERED);

				testMenuText1->reset();
				testMenuText1->translate(0.f, -925.f, 0.f);
				testMenuText1->scale(0.35f);
				testMenuText1->render(PROJECTION::ORTHOGRAPHIC, L"GAME WILL REBOOT WHEN COMPLETED!", ALIGNMENT::CENTERED);
			}
			else if (gameState.getGameState() == GameState::CurrentState::RESULTS) {
				if (gameState.results.size() <= 0) {
					continue;
				}

				// NOTE: THIS WHOLE SECTION IS JUST PLACEHOLDER UNTIL FINAL ART IS ADDED
				testMenuTitle->reset();
				testMenuTitle->translate(0.f, 900.f, 0.f);
				testMenuTitle->scale(0.5f);
				testMenuTitle->render(PROJECTION::ORTHOGRAPHIC, L"RESULTS", ALIGNMENT::CENTERED);

				testMenuText1->reset();
				testMenuText1->translate(0.f, 800.f, 0.f);
				testMenuText1->scale(0.5f);
				testMenuText1->render(PROJECTION::ORTHOGRAPHIC, gameState.results.back().getSong().getTitle(), ALIGNMENT::CENTERED);

				testMenuText1->reset();
				testMenuText1->translate(0.f, 700.f, 0.f);
				testMenuText1->scale(0.5f);
				testMenuText1->render(PROJECTION::ORTHOGRAPHIC, gameState.results.back().getSong().getArtist(), ALIGNMENT::CENTERED);

				testMenuText1->reset();
				testMenuText1->translate(0.f, 600.f, 0.f);
				testMenuText1->scale(0.5f);
				testMenuText1->render(PROJECTION::ORTHOGRAPHIC, L"Difficulty: " + to_wstring(gameState.results.back().getDifficulty()), ALIGNMENT::CENTERED);

				testMenuText1->reset();
				testMenuText1->translate(0.f, 500.f, 0.f);
				testMenuText1->scale(0.5f);
				testMenuText1->render(PROJECTION::ORTHOGRAPHIC, L"Score: " + to_wstring(gameState.results.back().getScore()), ALIGNMENT::CENTERED);

				testMenuText1->reset();
				testMenuText1->translate(0.f, 400.f, 0.f);
				testMenuText1->scale(0.5f);
				testMenuText1->render(PROJECTION::ORTHOGRAPHIC, L"Perfects: " + to_wstring(gameState.results.back().getPerfectCount()), ALIGNMENT::CENTERED);

				testMenuText1->reset();
				testMenuText1->translate(0.f, 300.f, 0.f);
				testMenuText1->scale(0.5f);
				testMenuText1->render(PROJECTION::ORTHOGRAPHIC, L"Nears: " + to_wstring(gameState.results.back().getNearCount()), ALIGNMENT::CENTERED);

				testMenuText1->reset();
				testMenuText1->translate(0.f, 200.f, 0.f);
				testMenuText1->scale(0.5f);
				testMenuText1->render(PROJECTION::ORTHOGRAPHIC, L"Miss: " + to_wstring(gameState.results.back().getMissCount()), ALIGNMENT::CENTERED);

				testMenuText1->reset();
				testMenuText1->translate(0.f, -400.f, 0.f);
				testMenuText1->scale(0.5f);
				testMenuText1->render(PROJECTION::ORTHOGRAPHIC, L"Press Start To Continue...", ALIGNMENT::CENTERED);
			}
			else if (gameState.getGameState() == GameState::CurrentState::FINAL_RESULTS) {
				// NOTE: THIS WHOLE SECTION IS JUST PLACEHOLDER UNTIL FINAL ART IS ADDED
				testMenuTitle->reset();
				testMenuTitle->translate(0.f, 900.f, 0.f);
				testMenuTitle->scale(0.5f);
				testMenuTitle->render(PROJECTION::ORTHOGRAPHIC, L"Final Results", ALIGNMENT::CENTERED);

				for (size_t i = 0; i < gameState.results.size(); i++) {
					testMenuText1->reset();
					testMenuText1->translate(0.f, 800.f - ((float)i * 400.f), 0.f);
					testMenuText1->scale(0.4f);
					testMenuText1->render(PROJECTION::ORTHOGRAPHIC, gameState.results[i].getSong().getTitle() + L" - " + gameState.results.back().getSong().getArtist(), ALIGNMENT::CENTERED);

					testMenuText1->reset();
					testMenuText1->translate(0.f, 600.f - ((float)i * 400.f), 0.f);
					testMenuText1->scale(0.5f);
					testMenuText1->render(PROJECTION::ORTHOGRAPHIC, to_wstring(gameState.results[i].getDifficulty()) + L" - " + to_wstring(gameState.results.back().getScore()), ALIGNMENT::CENTERED);
				}
			}
			else if (gameState.getGameState() == GameState::CurrentState::LOGIN_DETAILS) {
				DisplayName->render(PROJECTION::ORTHOGRAPHIC, userData.getDisplayName(), ALIGNMENT::LEFT, 0.f, 0.f, 0.f);
				UserTitle->render(PROJECTION::ORTHOGRAPHIC, userData.getTitle(), ALIGNMENT::LEFT, 0.f, 0.f, 0.f, .65f);

				Level->reset();
				Level->translate(950.f, -75.f, 0.f);
				Level->scale(0.75f);
				Level->render(PROJECTION::ORTHOGRAPHIC, L"Level", ALIGNMENT::CENTERED, 0.f, 0.f, 0.f);

				Level->translate(0.f, -100.f, 0.f);
				Level->scale(0.65f);
				Level->render(PROJECTION::ORTHOGRAPHIC, to_string(userData.getLevel()), ALIGNMENT::CENTERED, 0.f, 0.f, 0.f);

				PlayCount->reset();
				PlayCount->translate(950.f, -300.f, 0.f);
				PlayCount->scale(0.75f);
				PlayCount->render(PROJECTION::ORTHOGRAPHIC, L"Play Count", ALIGNMENT::CENTERED, 0.f, 0.f, 0.f);

				PlayCount->translate(0.f, -100.f, 0.f);
				PlayCount->scale(0.65f);
				PlayCount->render(PROJECTION::ORTHOGRAPHIC, to_string(userData.getPlayCount()), ALIGNMENT::CENTERED, 0.f, 0.f, 0.f);
			}
		}

		// Closed Curtains For Transitions
		glUseProgram(spriteShader.getProgram());
		{
			if (gameState.getGameState() != GameState::CurrentState::STARTUP) {
				ClosedCurtainLeft->update(currentOffset.count());
				ClosedCurtainLeft->render(PROJECTION::ORTHOGRAPHIC);

				ClosedCurtainRight->update(currentOffset.count());
				ClosedCurtainRight->render(PROJECTION::ORTHOGRAPHIC);
			}
		}

		// Display to the window
		gameWindow->display();

		// If all checks complete for startup
		if (gameState.getGameState() == GameState::CurrentState::STARTUP && allChecksComplete == true) {
			// Sleep for two seconds so the operator can see the status of the screen before it shifts to the next
			std::chrono::milliseconds timespan(1000);
			this_thread::sleep_for(2 * timespan);

			if (updateCheckStatus == 1) {
				// Since there is an update available, go to the updates screen
				gameState.setGameState(GameState::CurrentState::UPDATES);
			}
			else {
				// Otherwise, proceed as normal and go to the title screen
				gameState.setGameState(GameState::CurrentState::TITLE_SCREEN);
			}

			// Clear up all the memory for the text used on the startup screen as it won't be needed anymore
			delete gameVersion;
			delete startupText;
			delete networkText;
			delete colon1;
			delete updatesText;
			delete colon2;
			delete networkStatusText;
			delete updatesStatusText;
		}
	}

	// When not in the render loop, we don't need control of the game window
	gameWindow->setActive(false);

	// Makes sure that the startup thread finished before terminating this thread
	if (startup.joinable()) {
		startup.join();
	}

	// Make sure that the update thread is also completed before terminating this thread
	if (update.joinable()) {
		update.join();
	}
}

/**
 * Increase the test menu position by 1.
 *
 */
void ScreenRenderer::testMenuPosPlus() {
	this->testMenuPos++;
	if (this->testMenuPos > (this->testMenuTotalOptions - 1)) {
		this->testMenuPos = 0;
	}
}

/**
 * Decrease the test menu position by 1.
 *
 */
void ScreenRenderer::testMenuPosMinus() {
	this->testMenuPos--;
	if (this->testMenuPos < 0) {
		this->testMenuPos = this->testMenuTotalOptions - 1;
	}
}

/**
 * Reset the position of the test menus.
 *
 */
void ScreenRenderer::testMenuReset() {
	this->testMenuPos = 0;
	this->testMenuIOChkPos = 0;
	this->testMenuNetworkingPos = 0;
}

/**
 * Get the test menu position.
 *
 * @return the position of the test menu
 */
int ScreenRenderer::getTestMenuPos() {
	return this->testMenuPos;
}

/**
 * Inscrease io check menu by 1.
 *
 */
void ScreenRenderer::testMenuIOCheckPosPlus() {
	this->testMenuIOChkPos++;
	if (this->testMenuIOChkPos > (this->testMenuIOCheckTotalOptions - 1)) {
		this->testMenuIOChkPos = 0;
	}
}

/**
 * Decrease io check menu by 1.
 *
 */
void ScreenRenderer::testMenuIOCheckPosMinus() {
	this->testMenuIOChkPos--;
	if (this->testMenuIOChkPos < 0) {
		this->testMenuIOChkPos = this->testMenuIOCheckTotalOptions - 1;
	}
}

/**
 * Get the position of the io check menu.
 *
 * @return io check menu position
 */
int ScreenRenderer::getTestMenuIOCheckPos() {
	return this->testMenuIOChkPos;
}

/**
 * Increase the sound options menu by 1.
 *
 */
void ScreenRenderer::testMenuSoundOptionsPosPlus() {
	this->testMenuSoundOptionsPos++;
	if (this->testMenuSoundOptionsPos > (this->testMenuSoundOptionsTotalOptions - 1)) {
		this->testMenuSoundOptionsPos = 0;
	}
}

/**
 * Decrease the sound options menu by 1.
 *
 */
void ScreenRenderer::testMenuSoundOptionsPosMinus() {
	this->testMenuSoundOptionsPos--;
	if (this->testMenuSoundOptionsPos < 0) {
		this->testMenuSoundOptionsPos = this->testMenuSoundOptionsTotalOptions - 1;
	}
}

/**
 * Get the position of the sound options menu.
 *
 * @return sound options menu position
 */
int ScreenRenderer::getTestMenuSoundOptionsPos() {
	return this->testMenuSoundOptionsPos;
}

/**
 * Get the state of the sound options selected value.
 *
 * @return state of the sound options selected value
 */
bool ScreenRenderer::getTestMenuSoundOptionsSelected() {
	return this->testMenuSoundOptionsSelected;
}

/**
 * Set the sound options value.
 *
 * @param newValue the new state
 */
void ScreenRenderer::setTestMenuSoundOptionsSelected(bool newValue) {
	this->testMenuSoundOptionsSelected = newValue;
}

/**
 * Increase the networking menu by 1.
 *
 */
void ScreenRenderer::testMenuNetworkingPosPlus() {
	this->testMenuNetworkingPos++;
	if (this->testMenuNetworkingPos > (this->testMenuNetworkingTotalOptions - 1)) {
		this->testMenuNetworkingPos = 0;
	}
}

/**
 * Decrease the networking menu by 1.
 *
 */
void ScreenRenderer::testMenuNetworkingPosMinus() {
	this->testMenuNetworkingPos--;
	if (this->testMenuNetworkingPos < 0) {
		this->testMenuNetworkingPos = this->testMenuNetworkingTotalOptions - 1;
	}
}

/**
 * Get the option being hovered over.
 *
 * @return position of the item
 */
int ScreenRenderer::getTestMenuNetworkingPos() {
	return this->testMenuNetworkingPos;
}

/**
 * Update the song hovered over based on the amount the wheel changed.
 *
 * @param value amount wheel changed
 */
void ScreenRenderer::updateWheelRelation(int value) {
	this->wheelRelation += value;

	// <0 | Change Page Backwards (-1) and set wheelRelation to 290 to give a buffer
	//0 - 50 | First Song Selected
	//51 - 100 | Second Song Selected
	//101 - 150 | Third Song Selected
	//151 - 200 | Fourth Song Selected
	//201 - 250 | Fifth Song Selected
	//251 - 300 | Sixth Song Selected
	// >300 | Change Page Forwards (1) and set whgeelRelation to 10 to give a buffer
	int before = this->songSelectHoverOver;

	if (this->wheelRelation < 0) {
		this->wheelRelation = 290;
		this->songSelectHoverOver = 5;
		changeSongPage(-1);
	}
	else if (this->wheelRelation >= 0 && this->wheelRelation < 51) {
		this->songSelectHoverOver = 0;
	}
	else if (this->wheelRelation >= 51 && this->wheelRelation < 101) {
		this->songSelectHoverOver = 1;
	}
	else if (this->wheelRelation >= 101 && this->wheelRelation < 151) {
		this->songSelectHoverOver = 2;
	}
	else if (this->wheelRelation >= 151 && this->wheelRelation < 201) {
		this->songSelectHoverOver = 3;
	}
	else if (this->wheelRelation >= 201 && this->wheelRelation < 251) {
		this->songSelectHoverOver = 4;
	}
	else if (this->wheelRelation >= 251 && this->wheelRelation < 301) {
		this->songSelectHoverOver = 5;
	}
	else if (this->wheelRelation >= 301) {
		this->wheelRelation = 10;
		this->songSelectHoverOver = 0;
		changeSongPage(1);
	}

	if (before != this->songSelectHoverOver) {
		this->difficultyHoverOver = 0;
	}
}

/**
 * Change the difficulty selected.
 *
 * @param direction to change the hover over
 */
void ScreenRenderer::changeDifficultySelected(int direction) {
	//1 = advance difficulty forward ->
	//-1 = advance difficulty backwards <-

	this->difficultyHoverOver += direction;
	if (this->difficultyHoverOver < 0) {
		this->difficultyHoverOver = this->currentPageSongs[this->songSelectHoverOver].getNumberOfDifficulties() - 1;
	}
	else if (this->difficultyHoverOver >= this->currentPageSongs[this->songSelectHoverOver].getNumberOfDifficulties()) {
		this->difficultyHoverOver = 0;
	}
}

/**
 * Checks if the song is currently valid to play.
 *
 * @return true if song is valid
 */
bool ScreenRenderer::isCurrentSongValidToPlay() {
	return this->currentPageSongs[this->songSelectHoverOver].isSongValid();
}

/**
 * Get the song being hovered over.
 *
 * @return song hovered over
 */
int ScreenRenderer::getSongSelectHoverOver() {
	return this->songSelectHoverOver;
}

/**
 * Get the difficulty being hovered over.
 *
 * @return difficulty being hovered over
 */
int ScreenRenderer::getDifficultyHoverOver() {
	return this->difficultyHoverOver;
}

/**
 * Reset the screen renderer.
 *
 */
void ScreenRenderer::reset() {

	this->songSelectHoverOver = 0;
	this->wheelRelation = 25;
	this->currentSongPage = 1;
	this->difficultyHoverOver = 0;
}

/**
 * Change the song page.
 *
 * @param direction direction to change the page
 */
void ScreenRenderer::changeSongPage(int direction) {
	//1 = advance page forward ->
	//-1 = advance page backwards <-

	this->currentSongPage += direction;
	if (this->currentSongPage <= 0) { //If it goes to 0 or less, put it on the max page
		this->currentSongPage = this->numOfSongPages;
	}
	else if (this->currentSongPage > this->numOfSongPages) { //If it goes above max number of pages, put it back to first page
		this->currentSongPage = 1;
	}

	this->currentPageSongs.clear();
	int startingPoint = (this->currentSongPage - 1) * 6;
	for (int i = 0; i < 6; i++) {
		this->currentPageSongs.push_back(this->songs[(startingPoint + i)]);
	}

	logger.log(L"Switching Song Select to Page: " + to_wstring(this->currentSongPage));
}

void execStartupChecks() {
	// NETWORK
	// Set to 0 to indicate starting checking
	networkStatus = 0;

	// Actually perform the check
	networkStatus = network.checkConnection();

	// Set the gamestate based on the network result
	switch (networkStatus) {
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

	// UPDATES
	if (networkStatus == 1) {
		// Network Online - Perform the check now
		updateCheckStatus = 0;

		// Actually perform the check
		newVersion = network.checkForUpdates();

		if (newVersion == network.getLocalVersion()) {
			// No new updates
			updateCheckStatus = 2;
		}
		else {
			// Update available
			updateCheckStatus = 1;
		}
	}
	else {
		updateCheckStatus = -1;
	}

	// End by saying that all checks are complete
	logger.log(L"All Startup Checks Complete.");
	allChecksComplete = true;
}

void execDownloadUpdate() {
	// Set var to 0 saying download is starting
	updateDownloadStatus = 0;

	logger.log(L"Starting update download...");
	if (!network.downloadUpdate()) {
		// The update fails to download
		logger.logError(L"Failed to download update file!");
		//TODO: send to error code screen - non fatal instead of title screen first
		gameState.setGameState(GameState::CurrentState::TITLE_SCREEN);
		return;
	}
	logger.log(L"Update download complete.");

	// Update has finished downloading, next step is to extract it
	updateDownloadStatus = 1;

	logger.log(L"Beginning Update .ZIP Extraction...");

	HZIP hz;

	hz = OpenZip(_T("C:/SNA_UPDATE/Update.zip"), 0);
	SetUnzipBaseDir(hz, _T("C:/"));
	ZIPENTRY ze;
	GetZipItem(hz, -1, &ze);
	int numItems = ze.index;
	for (int zi = 0; zi < numItems; zi++) {
		GetZipItem(hz, zi, &ze);
		UnzipItem(hz, zi, ze.name);
	}
	CloseZip(hz);
	logger.log(L"Update .ZIP File Extracted.");

	// Update done extracting, next step is to modify startup batch file
	updateDownloadStatus = 2;

	logger.log(L"Modifying Startup Files...");

	std::ofstream ofs("C:/Rhythm.bat", std::ios::out | std::ios::trunc); // clear contents

	string startCommand = "start \"SNA\" /d C:\\" + newVersion + "\\ \"Sonataria.exe\"";

	ofs << startCommand;

	ofs.close();

	std::ofstream ofs2("C:/old_version.txt", std::ios::out | std::ios::trunc); // clear contents

	string version = network.getLocalVersion();

	ofs2 << version;

	ofs2.close();

	logger.log(L"Startup Modified.");

	// Startup modified, next step is to restart and launch the new game
	logger.log(L"Restarting Game As New Version...");
	system("start \"SNA\" /d C:\\ \"Rhythm.bat\"");

	gameState.setGameState(GameState::CurrentState::SHUTDOWN);
}

/**
 * Starts the game renderer.
 *
 * @param window the game window
 */
void rThread(sf::RenderWindow* window) {
	GameRenderer gameRenderer;
	gameRenderer.render(window);
}

void checkForProfile() {
	// Grab the card ID to check for a profile
	string cardID = RFIDCardReader::getCardReader()->getLastCardData();

	// Clear out the last user data if any
	userData.clearData();

	// Attempt to get the profile data | Set login complete when done
	// IF SUCCESS set loginComplete = 1
	// IF FAILED set loginComplete = 2
	if (network.GetProfileData(cardID)) {
		// Grabbed the profile data successfully
		screenRenderer.loginComplete = 1;
	}
	else {
		screenRenderer.loginComplete = 2;
	}
	
	// Clear the card out of the RFID Reader
	RFIDCardReader::getCardReader()->clearLastCardData();
}

void ScreenRenderer::ToggleCurtains(bool open) {
	if (open) {
		ClosedCurtainLeft->addInterpolation(INTERP_LINEAR, PROP_POSITION, Vector3(0.f, 0.f, 0.f), Vector3(-2.f, 0.f, 0.f), (float)gameState.CurtainTransitionTime);
		ClosedCurtainRight->addInterpolation(INTERP_LINEAR, PROP_POSITION, Vector3(0.f, 0.f, 0.f), Vector3(2.f, 0.f, 0.f), (float)gameState.CurtainTransitionTime);
	} else {
		ClosedCurtainLeft->addInterpolation(INTERP_LINEAR, PROP_POSITION, Vector3(-2.f, 0.f, 0.f), Vector3(0.f, 0.f, 0.f), (float)gameState.CurtainTransitionTime);
		ClosedCurtainRight->addInterpolation(INTERP_LINEAR, PROP_POSITION, Vector3(2.f, 0.f, 0.f), Vector3(0.f, 0.f, 0.f), (float)gameState.CurtainTransitionTime);
	}
}

int ScreenRenderer::getSongHoverOver() {
	return this->songSelectHoverOver;
}

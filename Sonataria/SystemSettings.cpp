#include <fstream>
#include <string>
#include "SystemSettings.h"
#include <iostream>
#include <vector>
#include "WindowsAudio.h"
#include "Logger.h"

SystemSettings systemSettings;

// Forward Declarations
void tokenize3(std::string const& str, const char delim, std::vector<std::string>& out);

/**
 * Default constructor.
 * 
 */
SystemSettings::SystemSettings() {
	this->windowsAudioLevel = 0.0f;
}

/**
 * Default deconstructor.
 * 
 */
SystemSettings::~SystemSettings() {

}

/**
 * Check if settings exist and if they don't create a file and initialize values to defaults.
 * 
 */
void SystemSettings::initSettings() {

	ifstream inFile("Settings/SysSettings.txt");

	// If the settings file exists and was open, load it
	if (inFile.is_open()) {
		logger.log(L"Settings file exists.  Reading contents...");

		// Create some holder variables
		vector<string> input;
		string line;

		while (getline(inFile, line)) {
			input.push_back(line);
		}

		inFile.close();

		// handle the lines from the vector
		// Loop for the amount of lines in that vector
		for (size_t i = 0; i < input.size(); i++) {
			const char delim = '|';

			// Break the line into the parts based on the delim char
			std::vector<std::string> out;
			tokenize3(input[i], delim, out);

			// Based on the type of data switch to what to set
			if (out[0] == "WIN-AUDIO") {
				this->windowsAudioLevel = stof(out[1]);
			}
			
		}

		this->setAllSettings();
	}
	// Create the file, store default values from constructor to file and set the default settings
	else {
		logger.log(L"No Settings file found.  Creating new one...");

		// Set defaults for System Settings here
		{
			this->windowsAudioLevel = 0.5f;
		}

		this->setAllSettings();
		this->saveSettingsToFile();
	}

	logger.log(L"Settings initialized.");
}

/**
 * Take settings stored in this object and save them to the file.
 * 
 */
void SystemSettings::saveSettingsToFile() {
	logger.log(L"Saving settings to file...");

	ofstream outFile("Settings/SysSettings.txt", ios::trunc);

	// Write settings here
	{
		outFile << "WIN-AUDIO|" << this->windowsAudioLevel << endl;
	}

	outFile.close();

	logger.log(L"Saving settings to file complete.");
}

/**
 * Go through each setting and set them where needed.
 * 
 */
void SystemSettings::setAllSettings() {

	// Helper Objects to set settings
	WindowsAudio winAudio;

	// Set Windows Audio
	winAudio.SetSystemVolume(this->windowsAudioLevel, WindowsAudio::VolumeUnit::Scalar);

	// SET OTHER SETTINGS HERE
}

/**
 * Set a specific setting value and then update the settings file.
 * 
 * @param setting
 * @param value
 */
void SystemSettings::updateSetting(Setting setting, float value) {
	switch (setting) {
		case Setting::WIN_AUDIO:
			this->windowsAudioLevel = value;
			break;

	}

	this->saveSettingsToFile();
}

/**
 * Break a line up based on a delim character.
 *
 * @param str string to split up
 * @param delim char to split based on
 * @param out output vector to store to
 */
void tokenize3(std::string const& str, const char delim, std::vector<std::string>& out) {

	size_t start;
	size_t end = 0;

	while ((start = str.find_first_not_of(delim, end)) != std::string::npos)
	{
		end = str.find(delim, start);
		out.push_back(str.substr(start, end - start));
	}
}
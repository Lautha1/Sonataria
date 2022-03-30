#include "UserData.h"
#include "Logger.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

UserData userData;

/**
 * Default constructor.
 * 
 */
UserData::UserData() {
	// Used to initialize the values
	clearData();
}

/**
 * Default deconstructor.
 * 
 */
UserData::~UserData() {

}

/**
 * Set the data for a user.
 * 
 * @param cardNum the card number for the account
 * @param dispName the display name on the account
 */
void UserData::setUserData(string data) {
	logger.log(data);

	json j = json::parse(data);

	// Check to see if new user
	this->NewUser = j["NewUser"];

	if (this->NewUser) {
		// New User

		// All the data we have for a new user is their card number | Can use defaults for the rest
		// DISPLAY NAME - Is set in the creation game state when the user enters their name
		this->CardNumber = j["CardNumber"];
		this->Title = "Newcomer";
		this->Level = 1;
		this->EXP = 0;
		this->PlayCount = 0;

		// Set Game Settings
		this->UseCustomSpeed = false;
		this->Speed = 300;
	}
	else {
		// Existing User

		this->DisplayName = j["DisplayName"];
		this->CardNumber = j["CardNumber"];
		this->Title = j["Title"];
		this->Level = j["Level"];
		this->EXP = j["EXP"];
		this->PlayCount = j["PlayCount"];

		// Set Game Settings
		this->UseCustomSpeed = j["GameSettings"]["UseCustomSpeed"];
		this->Speed = j["GameSettings"]["Speed"];
	}

	logger.log("User Data Set.");
}

/**
 * Clear the stored saved data.
 * 
 */
void UserData::clearData() {
	// Profile Information
	this->DisplayName = "GUEST";
	this->CardNumber = "";
	this->Title = "Newcomer";
	this->Level = 1;
	this->EXP = 0;
	this->PlayCount = 0;

	// Game Settings
	this->UseCustomSpeed = false;
	this->Speed = 300;

	// Validity
	this->NewUser = true;
}

/**
 * Check to see if the data is valid
 * 
 * @return 
 */
bool UserData::isNewUser() {
	return this->NewUser;
}

string UserData::getDisplayName() {
	return this->DisplayName;
}

bool UserData::useCustomSpeed() {
	return this->UseCustomSpeed;
}

int UserData::getGameSpeed() {
	return this->Speed;
}

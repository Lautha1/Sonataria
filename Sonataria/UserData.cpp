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

	// TODO: CHECK TO MAKE SURE NOT NEW USER FIRST
	// SET NOT VALID USE IF NEW USER

	// Set the User Data
	this->DisplayName = j["DisplayName"];
	this->CardNumber = j["CardNumber"];
	this->Title = j["Title"];
	this->Level = j["Level"];
	this->EXP = j["EXP"];
	this->PlayCount = j["PlayCount"];

	// Set Game Settings
	this->UseCustomSpeed = j["GameSettings"]["UseCustomSpeed"];
	this->Speed = j["GameSettings"]["Speed"];

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
}

/**
 * Check to see if there is actually a user set.
 * 
 * @return 
 */
bool UserData::isValidUser() {
	if (this->DisplayName != "" && this->CardNumber != "") {
		return true;
	}
	return false;
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

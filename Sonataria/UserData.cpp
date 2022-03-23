#include "UserData.h"

UserData userData;

/**
 * Default constructor.
 * 
 */
UserData::UserData() {
	this->DisplayName = "";
	this->CardNumber = -1;
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
void UserData::setUserData(int cardNum, string dispName) {
	this->CardNumber = cardNum;
	this->DisplayName = dispName;
}

/**
 * Clear the stored saved data.
 * 
 */
void UserData::clearData() {
	this->DisplayName = "";
	this->CardNumber = -1;
}

/**
 * Check to see if there is actually a user set.
 * 
 * @return 
 */
bool UserData::isValidUser() {
	if (this->DisplayName != "" && this->CardNumber != -1) {
		return true;
	}
	return false;
}

string UserData::getDisplayName() {
	return this->DisplayName;
}

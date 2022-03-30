#include <string>
#pragma once
using namespace std;

/**
 * Information to store the user data from the current session.
 */
class UserData {

	public:
		UserData();
		~UserData();
		void clearData();
		void setUserData(string);
		
		bool isNewUser();
		string getDisplayName();

		bool useCustomSpeed();
		int getGameSpeed();

	private:
		// Profile Information
		string DisplayName;
		string CardNumber;
		string Title;
		int Level;
		int EXP;
		int PlayCount;

		// Game Settings
		bool UseCustomSpeed;
		int Speed;

		// Validity
		bool NewUser;
};

extern UserData userData;
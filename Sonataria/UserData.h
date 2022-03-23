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
		void setUserData(int, string);
		
		bool isValidUser();
		string getDisplayName();

	private:
		string DisplayName;
		int CardNumber;
};

extern UserData userData;
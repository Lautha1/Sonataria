/**
 * @file Networking.h
 *
 * @brief Networking
 *
 * @author Julia Butenhoff
 */
#include <string>
using namespace std;
#pragma once

/**
 * Takes care of online features and connecting to the server
 */
class Networking {

	public:
		Networking();
		~Networking();

		string getLocalVersion();

		bool GetProfileData(string);

		int checkConnection();
		string checkForUpdates();
		bool downloadUpdate();
		

	private:
		string version;
		string ServerAddress;
};

extern Networking network;

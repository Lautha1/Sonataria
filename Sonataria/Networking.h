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
		int checkConnection();
		string checkForUpdates();
		string getLocalVersion();
		bool downloadUpdate();

	private:
		string version;
};

extern Networking network;

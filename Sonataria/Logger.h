#include <string>
#pragma once
using namespace std;

class Logger {

	private:


	public:
		Logger();
		~Logger();

		void log(wstring);
		void logError(wstring);

		void log(string);
		void logError(string);
};

extern Logger logger;
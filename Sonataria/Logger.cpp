#include "Logger.h"
#include <Windows.h>
#include <iostream>
#include <io.h>
#include <fcntl.h>
#include <locale.h>
#include <wincon.h>
using namespace std;

Logger logger;

/**
 * Base constructor.  Handles console setup for japanese characters
 * 
 */
Logger::Logger() {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

    char* a = setlocale(LC_ALL, "japanese");
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    CONSOLE_FONT_INFOEX fontInfo;
    fontInfo.cbSize = sizeof(fontInfo);
    fontInfo.FontFamily = 54;
    fontInfo.FontWeight = 400;
    fontInfo.nFont = 0;
    const wchar_t myFont[] = L"KaiTi";
    fontInfo.dwFontSize = { 18, 41 };
    std::copy(myFont, myFont + (sizeof(myFont) / sizeof(wchar_t)), fontInfo.FaceName);

    SetCurrentConsoleFontEx(hConsole, false, &fontInfo);

	cached_content = L"";
}

/**
 * Default deconstructor.
 * 
 */
Logger::~Logger() {

}

void Logger::log()
{
	wcout << L"[LOG]  | " << cached_content << endl;
    cached_content = L"";
}

void Logger::logError()
{
    wcerr << L"[WARN] | " << cached_content << endl;
	cached_content = L"";
}

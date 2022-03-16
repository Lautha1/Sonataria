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
}

/**
 * Default deconstructor.
 * 
 */
Logger::~Logger() {

}

/**
 * Output a message as a wide string.
 * 
 * @param content the string to output
 */
void Logger::log(wstring content) {
    wcout << L"[LOG]  | " << content << endl;
}

/**
 * Output a error message as a wide string.
 * 
 * @param content the string to output
 */
void Logger::logError(wstring content) {
    wcerr << L"[WARN] | " << content << endl;
}

/**
 * Output a message as a narrow string.
 *
 * @param content the string to output
 */
void Logger::log(string content) {
    wstring wContent;
    wContent.assign(content.begin(), content.end());
    wcout << L"[LOG]  | " << wContent << endl;
}

/**
 * Output a error message as a narrow string.
 *
 * @param content the string to output
 */
void Logger::logError(string content) {
    wstring wContent;
    wContent.assign(content.begin(), content.end());
    wcerr << L"[WARN] | " << wContent << endl;
}

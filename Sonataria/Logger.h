#pragma once

#include <string>

class Logger {
	private:
		std::wstring cached_content;
		void log();
		void logError();

	public:
		Logger();
		~Logger();

		template <typename... Types>
		void log(std::wstring content, Types... rest);

		template <typename... Types>
		void log(std::string content, Types... rest);

		template <typename... Types>
		void logError(std::wstring content, Types... rest);

		template <typename... Types>
		void logError(std::string content, Types... rest);
};

extern Logger logger;


/**
 * Output a message as a wide string.
 * 
 * @param content the string to output
 */
template <typename... Types>
void Logger::log(std::wstring content, Types... rest) {
	cached_content += content;

    log(rest...);
}

/**
 * Output a message as a wide string.
 * 
 * @param content the string to output
 */
template <typename... Types>
void Logger::log(std::string content, Types... rest) {
    std::wstring wContent;
    wContent.assign(content.begin(), content.end());
    cached_content += wContent; 

    log(rest...);
}


/**
 * Output a error message as a wide string.
 * 
 * @param content the string to output
 */
template <typename... Types>
void Logger::logError(std::wstring content, Types... rest)
{
	cached_content += content;

    logError(rest...);
}

/**
 * Output a error message as a narrow string.
 * 
 * @param content the string to output
 */
template <typename... Types>
void Logger::logError(std::string content, Types... rest)
{
    std::wstring wContent;
    wContent.assign(content.begin(), content.end());
    cached_content += wContent; 

    logError(rest...);
}

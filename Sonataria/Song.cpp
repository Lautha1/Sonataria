#define _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS
#include "Song.h"
#include <fstream>
#include <iostream>
#include <string>
#include "Logger.h"
#include <codecvt>
#include <locale>

// Forward Declaration
void tokenize(std::wstring const& str, const char delim, std::vector<std::wstring>& out);
string eraseAllSubStr(string mainStr, string toErase);
string ws2s(const std::wstring& wstr);

/**
 * Constructor for a song.
 * 
 */
Song::Song() {
	this->songID = -1;
	this->title = L"-";
	this->author = L"-";
	this->bpm = "-";
	this->jacketArt = "./Textures/MissingJacketArt.png";
	this->roundArt = "./Textures/MissingRoundArt.png";
	this->path = "";
	this->valid = false;
}

/**
 * Default deconstructor.
 * 
 */
Song::~Song() {

}

/**
 * Create a song reading in from an info.txt file.
 * 
 * @param path to the info file
 */
Song::Song(string path) {
	// Set the path to the file
	this->path = eraseAllSubStr(path, "info.txt");

	// Create some holder variables
	vector<wstring> input;
	wstring line;

	// Read from the text file into the vector
	wifstream inStream(path);

	inStream.imbue(std::locale(std::locale::empty(), new std::codecvt_utf8<wchar_t>));

	if (inStream.is_open()) {
		while (getline(inStream, line)) {
			input.push_back(line);
		}
		inStream.close();
	}
	else {
		logger.logError(L"Error opening file!");
	}

	// Loop for the amount of lines in that vector
	for (size_t i = 0; i < input.size(); i++) {
		const char delim = '-';

		// Break the line into the parts based on the delim char
		std::vector<std::wstring> out;
		tokenize(input[i], delim, out);

		// Based on the type of data switch to what to set
		if (out[0] == L"songid") {
			this->songID = stoi(out[1]);
		}
		else if (out[0] == L"title") {
			this->title = out[1];
		}
		else if (out[0] == L"author") {
			this->author = out[1];
		}
		else if (out[0] == L"bpm") {
			this->bpm = ws2s(out[1]);
		}
		else if (out[0] == L"jacketArt") {
			this->jacketArt = ws2s(out[1]);
		}
		else if (out[0] == L"audioFile") {
			this->audioFile = ws2s(out[1]);
		}
		else if (out[0] == L"roundArt") {
			this->roundArt = ws2s(out[1]);
		}
		else if (out[0] == L"diffs") {
			const char delim2 = '/';

			std::vector<std::wstring> out2;
			tokenize(out[1], delim2, out2);

			for (size_t j = 0; j < out2.size(); j++) {
				string diff = ws2s(out2[j]);
				Chart tmp(stoi(diff));
				this->charts.push_back(tmp);
			}
		}
	}

	// Set this is now a valid song
	this->valid = true;
}

string ws2s(const std::wstring& wstr)
{
	using convert_typeX = std::codecvt_utf8<wchar_t>;
	std::wstring_convert<convert_typeX, wchar_t> converterX;

	return converterX.to_bytes(wstr);
}

/**
 * Copy constructor for a song.
 * 
 * @param old_str song to copy from
 */
Song::Song(const Song& old_str) {
	this->valid = old_str.valid;
	this->songID = old_str.songID;
	this->title = old_str.title;
	this->author = old_str.author;
	this->bpm = old_str.bpm;
	this->charts = old_str.charts;
	this->jacketArt = old_str.jacketArt;
	this->roundArt = old_str.roundArt;
	this->path = old_str.path;
	this->audioFile = old_str.audioFile;
}

/**
 * Gets the path to the round art.
 * 
 * @return path to round art
 */
string Song::getRoundArtPath() {
	return this->path + this->roundArt;
}

/**
 * Break a line up based on a delim character.
 * 
 * @param str string to split up
 * @param delim char to split based on
 * @param out output vector to store to
 */
void tokenize(std::wstring const& str, const char delim, std::vector<std::wstring>& out) {

	size_t start;
	size_t end = 0;

	while ((start = str.find_first_not_of(delim, end)) != std::string::npos)
	{
		end = str.find(delim, start);
		out.push_back(str.substr(start, end - start));
	}
}

/**
 * Remove instances of string from another string.
 * 
 * @param mainStr main string to look through
 * @param toErase string to remove from main
 * @return modified string
 */
string eraseAllSubStr(string mainStr, string toErase) {

	size_t pos = std::string::npos;

	// Search for the substring in string in a loop untill nothing is found
	while ((pos = mainStr.find(toErase)) != std::string::npos)
	{
		// If found then erase it from string
		mainStr.erase(pos, toErase.length());
	}

	return mainStr;
}

/**
 * Gets the path to the jacket art of the song.
 * 
 * @return the jacket art path
 */
string Song::getJacketArtPath() {
	return this->path + this->jacketArt;
}

/**
 * Gets the path to the audio file.
 * 
 * @return path to audio file
 */
string Song::getAudioFilePath() {
	return this->path + this->audioFile;
}

/**
 * Gets the title of the song.
 * 
 * @return the song title
 */
wstring Song::getTitle() {
	return this->title;
}

/**
 * Gets the artist of the song.
 * 
 * @return the songs artist
 */
wstring Song::getArtist() {
	return this->author;
}

/**
 * Gets the BPM of the song.
 * 
 * @return the songs BPM
 */
string Song::getBPM() {
	return this->bpm;
}

/**
 * Gets the quantity of difficulties that song has.
 * 
 * @return quantity of difficulties
 */
int Song::getNumberOfDifficulties() {
	return this->charts.size();
}

/**
 * Get the difficulty number at position X.
 * 
 * @param position in the charts array
 * @return the difficulty number
 */
int Song::getDifficultyNumber(int position) {
	return this->charts[position].getDifficulty();
}

/**
 * Gets if the song is valid.
 * 
 * @return true is valid
 */
bool Song::isSongValid() {
	return this->valid;
}

/**
 * Gets the path to the song.
 * 
 * @return the path to the song
 */
string Song::getPath() {
	return this->path;
}
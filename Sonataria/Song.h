/**
 * @file Song.h
 *
 * @brief Song
 *
 * @author Julia Butenhoff
 */
#pragma once
#include "Chart.h"
#include <vector>
#include <string>
using namespace std;

/**
 * Contains file information regarding a song
 */
class Song {

	private:
		bool valid;
		int songID;
		wstring title;
		wstring author;
		string bpm;
		vector<Chart> charts;
		string jacketArt;
		string path;
		string audioFile;

	public:
		Song();
		Song(string);
		Song(const Song& old_obj);
		~Song();
		string getJacketArtPath();
		wstring getTitle();
		wstring getArtist();
		string getBPM();
		int getNumberOfDifficulties();
		int getDifficultyNumber(int);
		bool isSongValid();
		string getAudioFilePath();
		string getPath();
};
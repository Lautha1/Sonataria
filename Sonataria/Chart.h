/**
 * @file Chart.h
 *
 * @brief Chart
 *
 * @author Julia Butenhoff
 */
#pragma once

/**
 * Keeps track of the difficulty related to a chart
 */
class Chart {

	private:
		int difficulty;

	public:
		Chart(int);
		~Chart();
		int getDifficulty();
};
#include "Chart.h"

/**
 * Constructor for a chart.
 * 
 * @param diff The difficulty for the chart
 */
Chart::Chart(int diff) {
	this->difficulty = diff;
}

/**
 * Default Deconstructor.
 * 
 */
Chart::~Chart() {

}

/**
 * Gets the difficulty of the chart.
 * 
 * @return the difficulty of the chart
 */
int Chart::getDifficulty() {
	return this->difficulty;
}
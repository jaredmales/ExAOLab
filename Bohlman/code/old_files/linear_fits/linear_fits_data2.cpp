/*! \file linear_fit.cpp
\brief A documented file that uses y int and slope to correctly output coordinates at certain points
*/

#include <fstream>
#include <gsl/gsl_fit.h>
#include <iostream>
#include <vector>

//  Main function
/** Initializes pylon resources, takes pictures, closes all pylon resources.
* \return an integer: 0 upon exit success, 1 otherwise
*/
int main(int argc, ///< [in] the integer value of the count of the command line arguments
	char* argv[] ///< [ch.ar] the integer value of the count of the command line arguments
)
{
	int exitCode = 0;
	double b = 60.4323, m = 56.7046;
	std::vector <double> points(0);
	points.push_back(26);
	points.push_back(50);
	points.push_back(75);
	points.push_back(100);
	points.push_back(125);
	points.push_back(150);
	points.push_back(175);
	points.push_back(199);
	points.push_back(222);
	points.push_back(242);
	for (int i = 0; i < 10; i++) {
		std::cout << points[i] << '\t' << (points[i]*(1/m)) + b/m << std::endl;
	}
	return exitCode;
}

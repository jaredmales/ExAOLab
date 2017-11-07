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
	double b = 5.29139, m = 36.3025;
	std::vector <double> points(0);
	points.push_back(22);
	points.push_back(44);
	points.push_back(67);
	points.push_back(89);
	points.push_back(112);
	points.push_back(135);
	points.push_back(157);
	points.push_back(180);
	points.push_back(202);
	points.push_back(223);
	for (int i = 0; i < 10; i++) {
		std::cout << points[i] << '\t' << (points[i]*(1/m)) + b/m << std::endl;
	}
	return exitCode;
}

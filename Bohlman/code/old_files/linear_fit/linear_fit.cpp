/*! \file linear_fit.cpp
\brief A documented file that finds slope and y intercept of median values from data.out
*/

#include <fstream>
#include <gsl/gsl_fit.h>
#include <iostream>
#include <vector>

//  Main function
/** Reads in median mean and variance values, outputs the gain and read noise
* \return an integer: 0 upon exit success, 1 otherwise
*/
int main(int argc, ///< [in] the integer value of the count of the command line arguments
	char* argv[] ///< [ch.ar] the integer value of the count of the command line arguments
)
{
	int exitCode = 0;
	double n, m;
	int size = 0;
	std::vector <double> var(0);
	std::vector <double> mean(0);
	std::ifstream inFile;
	inFile.open("data.out");																		// Read in file of mean and variance medians
	if (!inFile) {																					// If file can't be opened
        	std::cerr << "Unable to open file" << std::endl;										// Throw an error
		exitCode = 1;
    	}
	else {																							// If file can be opened
		while(inFile>>n>>m){																		// While lines are read in that look like "mean   variance"
			mean.push_back(n);																		// Read in mean values
			var.push_back(m);																		// Read in variance values
			++size;
		}
	}
	if (size != 0) {																				// If more than zero values are read in
		//size = size - (size/2);
		double c0 = 0, c1 = 0, cov00 = 0, cov01 = 0, cov11 = 0, sumsq = 0;							// Calculate gain and read noise from a linear fit to the data
  		gsl_fit_linear(&mean[0], 1, &var[0], 1, size, &c0, &c1, &cov00, &cov01, &cov11, &sumsq);
		std::cout << "M (Gain): " << 1/c1 << '\n' << "B (Read Noise): " << c0/c1 << std::endl;
	}
	else {																							// If nothng was read in
        	std::cerr << "Nothing read in." << std::endl;											// Say nothing was read in
		exitCode = 1;
	}		
	return exitCode;
}

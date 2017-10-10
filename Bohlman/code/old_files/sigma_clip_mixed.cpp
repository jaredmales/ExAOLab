/*! \file sigma_clip_mixed.cpp
\brief A documented file that sigma clips and finds the median of already made files.
*/

#include "write_basler_fits.h"
#include <numeric>
#include <cmath>
#include <gsl/gsl_fit.h>

//  std_dev_calc function
/** Takes in a vector structure and finds the standard deviation of the data elements
* \return an integer
*/
double std_dev_calc(std::vector<double> v) {
	double mean = std::accumulate(v.begin(), v.end(), 0.0) / v.size();
	double std_dev = 0;
	for (int i = 0; i < v.size(); ++i) {
		//cout << v[i] << endl;
		std_dev = std_dev + ((mean - v[i]) * (mean - v[i]));
	}
	std_dev = std_dev / v.size();
	return std_dev;
}


//  Main function
/** Initializes pylon resources, takes pictures, closes all pylon resources.
* \return an integer: 0 upon exit success, 1 otherwise
*/
int main(int argc, ///< [in] the integer value of the count of the command line arguments
	char* argv[] ///< [ch.ar] the integer value of the count of the command line arguments
)
{
	int exitCode = 0;
	int expArray[c_countOfImagesToGrab];
	int i, exp = 600;
	for (i = 0; i < c_countOfImagesToGrab; i++) {
		if (i % 100 == 0 && i > 0)
			exp = exp + 575;
		expArray[i] = exp;
	}
	const char *names[c_countOfImagesToGrab];
	for (i = 0; i < c_countOfImagesToGrab; ++i) {  //creates an array of names for each of the files previously made
		char filename[40];
		strncpy(filename, "fitsimg_exp", sizeof(filename));
		char exp_str[10];
		sprintf(exp_str, "%d", expArray[i]);
		strcat(filename, exp_str);
		char num_str[10];
		sprintf(num_str, "_%d", i);
		strcat(filename, num_str);
		strcat(filename, ".fits");
		names[i] = strdup(filename);
	}
	std::vector<fitsfile*> fpt_arr(c_countOfImagesToGrab);		//Array of fits pointers
	for (i = 0; i < c_countOfImagesToGrab; ++i) {
		fitsfile* fptr;
		fpt_arr.push_back(fptr);
		if (fits_open_file(&(fpt_arr.at(i)), names[i], READONLY, &exitCode)) {
			fits_report_error(stderr, exitCode);
			exit(1);
		}
	}
	int width = 640, height = 480;
	int j, k;

	for (k = 1; k <= height; k = k+10) {   //Looks through each pixel in a picture
		for (j = 1; j <= width; j = j+10) {
			//cout << k << "   " << j << endl;
			int iterator = 0;
			for (i = 0; i < c_countOfImagesToGrab; ++i) {
				double pixel_arr[100];
				long fpixel[2] = { j,k };
				double pixels;
				if (fits_read_pix(fpt_arr.at(i), TDOUBLE, fpixel, 1, NULL, &pixels, NULL, &exitCode)) { //read a singular pixel
					fits_report_error(stderr, exitCode);  // Prints out any fits error messages
					exit(1);
				}
				pixel_arr[iterator] = pixels;	//put it in an array to be compared with all other pixels
				++iterator;
				if (iterator == 100) {
					std::vector<double> v(pixel_arr, pixel_arr + 100);
					double std_dev_num = std_dev_calc(v);
					double mean_num = std::accumulate(v.begin(), v.end(), 0.0) / v.size();
					cout << mean_num <<  '\t' << std_dev_num << endl;
					iterator = 0;
				}
			}
		}
	}

	for (i = 0; i < c_countOfImagesToGrab; ++i) {
		fits_close_file(fpt_arr.at(i), &exitCode);
	}
	return exitCode;
}

/*! \file sigma_clip_mixed.cpp
\brief A documented file that sigma clips and finds the median of already made files.
*/

#include "write_basler_fits.h"
#include <numeric>
#include <cmath>
#include <gsl/gsl_fit.h>

//  std_dev_calc function
/** Takes in a vector structure and finds the standard deviation of the data elements
* \return an integer: 0 upon exit success, 1 otherwise
*/
double std_dev_calc(std::vector<double> v) {
	double mean = 0;
	for (size_t ii = 0; ii < v.size(); ++ii) {
		mean = mean + v.at(ii);
	}
	mean = mean / v.size();
	double std_dev_arr[10];
	for (size_t ii = 0; ii < v.size(); ++ii) {
		std_dev_arr[ii] = (mean - v.at(ii)) * (mean - v.at(ii));
	}
	double std_dev = 0;
	for (size_t ii = 0; ii < v.size(); ++ii) {
		std_dev = std_dev + std_dev_arr[ii];
	}
	std_dev = std_dev / v.size();
	std_dev = sqrt(std_dev);
	return std_dev;
}

//  sigma_clip function
/** Takes in a vector structure and sigma clips it until exit criteria is reached.
* Is a recursive function.
* \return an integer: 0 upon exit success, 1 otherwise
*/
std::vector<double> sigma_clip(std::vector<double> v) {
	int size = v.size();
	std::nth_element(v.begin(), v.begin() + v.size() / 2, v.end());
	double median = v[v.size() / 2];
	double std_dev = std_dev_calc(v);
	vector<double> ::iterator it;

	it = std::remove_if(v.begin(), v.end(), std::bind2nd(greater<double>(), median + std_dev));
	v.erase(it, v.end());

	it = std::remove_if(v.begin(), v.end(), std::bind2nd(less<double>(), median - std_dev));
	v.erase(it, v.end());

	int new_size = v.size();
	if (new_size != size) {
		double new_std_dev = std_dev_calc(v);
		std::nth_element(v.begin(), v.begin() + v.size() / 2, v.end());
		median = v[v.size() / 2];
		double dev_factor;
		if (new_std_dev != 0)
			dev_factor = (std_dev - new_std_dev) / new_std_dev;
		else
			return v;

		if (new_std_dev < (std_dev - dev_factor))
			return v;
		else
			return sigma_clip(v);
	}
	else
		return v;
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
	std::vector<double> gain(640*480);		//Array of fits pointers
	std::vector<double> noise(640*480);

	for (k = 150; k <= 300; k = k+1) {   //Looks through each pixel in a picture
		for (j = 200; j <= 400; j = j+5) {
/*	for (k = 1; k <= height; ++k) {   //Looks through each pixel in a picture
		for (j = 1; j <= width; ++j) {*/
			double mean[10];
			double std_dev[10];
			int iterator = 0;
			int iterator2 = 0;
			for (i = 0; i < c_countOfImagesToGrab; ++i) {
				double pixel_arr[1000];
				long fpixel[2] = { j,k };
				double pixels;
				if (fits_read_pix(fpt_arr.at(i), TDOUBLE, fpixel, 1, NULL, &pixels, NULL, &exitCode)) { //read a singular pixel
					fits_report_error(stderr, exitCode);  // Prints out any fits error messages
					exit(1);
				}
				pixel_arr[iterator] = pixels;	//put it in an array to be compared with all other pixels
				++iterator;
				if (iterator == 100) {
					std::vector<double> v(pixel_arr, pixel_arr + 10);
					/*
					for (std::vector<double>::const_iterator i = v.begin(); i != v.end(); ++i)
    						std::cout << *i << ", ";	 
					cout << endl;
					*/
					double std_dev_num = std_dev_calc(v);
					double mean_num = std::accumulate(v.begin(), v.end(), 0.0) / v.size();
					cout << mean_num <<  '\t' << (std_dev_num*std_dev_num) << endl;
					mean[iterator2] = mean_num;
					std_dev[iterator2] = std_dev_num;
					++iterator2;
					iterator = 0;
				}
			}
			/*
			double c0 = 0, c1 = 0, cov00 = 0, cov01 = 0, cov11 = 0, sumsq = 0;
  			gsl_fit_linear(mean, 1, std_dev, 1, 10, &c0, &c1, &cov00, &cov01, &cov11, &sumsq);
			std::cout << "M (Gain): " << 1/c1 << '\n' << "B (Read Noise): " << c0/c1 << std::endl;
			gain.push_back(1/c1);
			noise.push_back(c0/c1);
			*/
		}
	}
	/*
	double gain_mean = std::accumulate(gain.begin(), gain.end(), 0.0)/gain.size();
	cout << "Gain: "<< gain_mean << endl;
	double noise_mean = std::accumulate(noise.begin(), noise.end(), 0.0)/noise.size();
	cout << "Read Noise: " << noise_mean << endl;
	*/
	for (i = 0; i < c_countOfImagesToGrab; ++i) {
		fits_close_file(fpt_arr.at(i), &exitCode);
	}
	return exitCode;
}

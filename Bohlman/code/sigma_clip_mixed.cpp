/*! \file sigma_clip_mixed.cpp
\brief A documented file that sigma clips and finds the median of already made files.
*/

#include "stdafx.h"
#include "write_basler_fits.h"

//  std_dev_calc function
/** Takes in a vector structure and finds the standard deviation of the data elements
* \return an integer: 0 upon exit success, 1 otherwise
*/
double std_dev_calc(std::vector<double> v) {
	int ii;
	double mean = 0;
	for (ii = 0; ii < v.size(); ++ii) {
		mean = mean + v.at(ii);
	}
	mean = mean / v.size();
	double std_dev_arr[10];
	for (ii = 0; ii < v.size(); ++ii) {
		std_dev_arr[ii] = (mean - v.at(ii)) * (mean - v.at(ii));
	}
	double std_dev = 0;
	for (ii = 0; ii < v.size(); ++ii) {
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
	int ii;
	int size = v.size();
	std::nth_element(v.begin(), v.begin() + v.size() / 2, v.end());
	double median = v[v.size() / 2];
	double std_dev = std_dev_calc(v);
	vector<double> ::iterator it;
	it = std::remove_if(v.begin(), v.end(), std::bind2nd(greater<double>(), median + std_dev));
	v.erase(it, v.end());

	it = std::remove_if(v.begin(), v.end(), std::bind2nd(greater<double>(), median - std_dev));
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
	int i, exp = 1000;
	for (i = 0; i < c_countOfImagesToGrab; i++) {
		if (i % 10 == 0 && i > 0)
			exp = exp + 1000;
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
		names[i] = filename;
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
	
	int iterator = 0, image_num = 0;
	for (iterator; iterator < 10; iterator++) {
		int width = 640, height = 480;
		double *image_arr = (double*)calloc(width * height, sizeof(double)); //Array of pixel values for final image
		int j, k;
		for (k = 1; k <= height; ++k) {   //Looks through each pixel in a picture
			for (j = 1; j <= width; ++j) {
				int ii;
				double pixel_arr[10];
				for (ii = 0; ii < 10; ++ii) { //For each image
					long fpixel[2] = { j,k };
					double pixels;
					if (fits_read_pix(fpt_arr.at(image_num), TDOUBLE, fpixel, 1, NULL, &pixels, NULL, &exitCode)) { //read a singular pixel
						fits_report_error(stderr, exitCode);  // Prints out any fits error messages
						exit(1);
					}
					pixel_arr[ii] = pixels;	//put it in an array to be compared with all other pixels
				}
				std::vector<double> v(pixel_arr, pixel_arr + 10);  
				v = sigma_clip(v); //sigma clip
				std::nth_element(v.begin(), v.begin() + v.size() / 2, v.end()); //Find median of all pixel values
				image_arr[(k - 1)*width + (j - 1)] = v[v.size() / 2];
			}
		}
		
		fitsfile *fptr;
		long naxes[2] = { width, height };
		long naxis = 2, fpixel = 1;

		char filename2[40];
		strncpy(filename2, "!sigmaclip_image", sizeof(filename2));
		char num_str[10];
		sprintf(num_str, "_%d", image_num);
		strcat(filename2, num_str);
		strcat(filename2, ".fits");

		if (fits_create_file(&fptr, filename2, &exitCode) != 0) { //Creates new fits file
			fits_report_error(stderr, exitCode);  // Prints out any fits error messages
			exit(1);
		}
		if (fits_create_img(fptr, LONGLONG_IMG, naxis, naxes, &exitCode) != 0) { //Creates the primary array image
			fits_report_error(stderr, exitCode);  // Prints out any fits error messages
			exit(1);
		}
		if (fits_write_img(fptr, TDOUBLE, fpixel, width*height, image_arr, &exitCode) != 0) {// Writes pointer values to the image
			fits_report_error(stderr, exitCode);  // Prints out any fits error messages
			exit(1);
		}
		if (fits_close_file(fptr, &exitCode) != 0) { // Closes the fits file
			fits_report_error(stderr, exitCode);  // Prints out any fits error messages
			exit(1);
		}
		free(image_arr);
		std::cout << "Median image at minimum exposure produced!" << endl; //We did it
		++image_num;
	}

	for (i = 0; i < c_countOfImagesToGrab; ++i) {
		fits_close_file(fpt_arr.at(i), &exitCode);
	}

	std::cout << "Finished with the program" << endl;
	cerr << endl << "Press Enter to exit." << endl;
	while (cin.get() != '\n');
	return exitCode;
}


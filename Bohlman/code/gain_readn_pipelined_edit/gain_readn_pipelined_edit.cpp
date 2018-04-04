/*! \file gain_readn_pipelined.cpp
\brief A documented file that sigma clips and finds the median of already made files.
*/

#include "write_basler_fits.h"
#include <numeric>
#include <cmath>
#include <fstream>
#include <iostream>
#include <vector>
#include <algorithm>
#include <gsl/gsl_fit.h>

//  std_dev_calc function
/** Takes in a vector and finds the standard deviation of the data elements
* \return an integer
*/
double std_dev_calc(std::vector<double> v) {
	double mean = std::accumulate(v.begin(), v.end(), 0.0) / v.size();		// Find mean of all elements: sum/size
	double std_dev = 0;														// Calculate the standard deviation of the elements
	for (int i = 0; i < v.size(); ++i) {
		//std::cout << v[i] << std::endl;
		std_dev = std_dev + ((mean - v[i]) * (mean - v[i]));
	}
	std_dev = std_dev / v.size();
	return std_dev;															// Return the std deviation
}

double inPlaceMedian( std::vector<double> v)
{
   std::vector<double> tmp = v;
   std::nth_element(tmp.begin(), tmp.begin() + tmp.size()/2, tmp.end());
   return tmp[tmp.size()/2];
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
	int expArray[c_countOfImagesToGrab];																									// Creates array of exposure times from the images used
	int i, exp = 60;
	int j = 0;
	for (i = 0; i < c_countOfImagesToGrab; i++) {
		if (i % 100 == 0 && i > 0)
			exp = exp + 10;
		expArray[i] = exp;
	}
	const char *names[c_countOfImagesToGrab];																								// Creates an array of names for each of the images that will be used
	for (i = 0; i < c_countOfImagesToGrab; ++i) {  																							
		char filename[100];
		strncpy(filename, "/home/cbohlman/Documents/caao_summer2017/ExAOLab/Bohlman/code/raw_data_040118/fitsimg_exp", sizeof(filename));
		char exp_str[10];
		sprintf(exp_str, "%d", expArray[i]);
		strcat(filename, exp_str);
		char num_str[10];

		if (j > 9) {
			j = 0;
		}
		sprintf(num_str, "_%d", j);
		j++;
		strcat(filename, num_str);
		strcat(filename, ".fits");
		names[i] = strdup(filename);
	}
	std::vector<fitsfile*> fpt_arr(c_countOfImagesToGrab);																					// Creates array of fits pointers that are opened according to the image name previously generated
	for (i = 0; i < c_countOfImagesToGrab; ++i) {
		fitsfile* fptr;
		fpt_arr.push_back(fptr);
		if (fits_open_file(&(fpt_arr.at(i)), names[i], READONLY, &exitCode)) {
			fits_report_error(stderr, exitCode);
			exit(1);
		}
	}

	std::vector <double> mean1(0);							// Set up mean and variance arrays
	std::vector <double> mean2(0);
	std::vector <double> mean3(0);
	std::vector <double> mean4(0);
	std::vector <double> mean5(0);
	std::vector <double> mean6(0);
	std::vector <double> mean7(0);
	std::vector <double> mean8(0);
	std::vector <double> mean9(0);
	std::vector <double> mean10(0);
	std::vector <double> variance1(0);
	std::vector <double> variance2(0);
	std::vector <double> variance3(0);
	std::vector <double> variance4(0);
	std::vector <double> variance5(0);
	std::vector <double> variance6(0);
	std::vector <double> variance7(0);
	std::vector <double> variance8(0);
	std::vector <double> variance9(0);
	std::vector <double> variance10(0);
	double n,m;
	int ii = 0;

	int width = 640, height = 480;
	int k;
	std::vector<double> array3D(width*height*c_countOfImagesToGrab);																		// Creates array of new data for 100 images

	long fpixel[2], lpixel[2], inc[2];
	fpixel[0] = fpixel[1] = 1;
    lpixel[0] = height;
    lpixel[1] = width;  
    inc[0] = inc[1] = 1;
	for (i = 0; i < c_countOfImagesToGrab; ++i) {
		if (fits_read_pix(fpt_arr.at(i), TDOUBLE, fpixel, width*height, NULL, &array3D[height*width*i], NULL, &exitCode)) {
			fits_report_error(stderr, exitCode);  // Prints out any fits error messages
			exit(1);
		}
	}
	
	// Get mean and variance of each image set, and put them into vectors
	
	for (k = 1; k <= height; k = k+10) {   //Looks through each pixel in a picture
		for (j = 1; j <= width; j = j+10) {
			double pixel_arr[10];
			
			int z = 0;
			for (i = 0; i < 10; ++i) {
				//printf("%d    %d     %d\n", i, j,k);
				pixel_arr[z] = array3D[((k-1)*width + (j-1))+(i*width*height)];
				z++;
			}
			std::vector<double> v01(pixel_arr, pixel_arr + 10);
			double std_dev_num = std_dev_calc(v01);
			double mean_num = std::accumulate(v01.begin(), v01.end(), 0.0) / v01.size();
			mean1.push_back(mean_num);
			variance1.push_back(std_dev_num);
			
			z = 0;
			for (i = 10; i < 20; ++i) {
				pixel_arr[z] = array3D[((k-1)*width + (j-1))+(i*width*height)];
				z++;
			}
			std::vector<double> v02(pixel_arr, pixel_arr + 10);
			std_dev_num = std_dev_calc(v02);
			mean_num = std::accumulate(v02.begin(), v02.end(), 0.0) / v02.size();
			//cout << mean_num <<  '\t' << std_dev_num << endl;
			mean2.push_back(mean_num);
			variance2.push_back(std_dev_num);
			
			z = 0;
			for (i = 20; i < 30; ++i) {
				pixel_arr[z] = array3D[((k-1)*width + (j-1))+(i*width*height)];
				z++;
			}
			std::vector<double> v03(pixel_arr, pixel_arr + 10);
			std_dev_num = std_dev_calc(v03);
			mean_num = std::accumulate(v03.begin(), v03.end(), 0.0) / v03.size();
			mean3.push_back(mean_num);
			variance3.push_back(std_dev_num);
			
			z = 0;
			for (i = 30; i < 40; ++i) {
				pixel_arr[z] = array3D[((k-1)*width + (j-1))+(i*width*height)];
				z++;
			}
			std::vector<double> v04(pixel_arr, pixel_arr + 10);
			std_dev_num = std_dev_calc(v04);
			mean_num = std::accumulate(v04.begin(), v04.end(), 0.0) / v04.size();
			cout << "v04 " << mean_num <<  '\t' << std_dev_num << endl;
			mean4.push_back(mean_num);
			variance4.push_back(std_dev_num);
			
			z = 0;
			for (i = 40; i < 50; ++i) {
				pixel_arr[z] = array3D[((k-1)*width + (j-1))+(i*width*height)];
				z++;
			}
			std::vector<double> v05(pixel_arr, pixel_arr + 10);
			std_dev_num = std_dev_calc(v05);
			mean_num = std::accumulate(v05.begin(), v05.end(), 0.0) / v05.size();
			mean5.push_back(mean_num);
			variance5.push_back(std_dev_num);

			z = 0;
			for (i = 50; i < 60; ++i) {
				pixel_arr[z] = array3D[((k-1)*width + (j-1))+(i*width*height)];
				z++;
			}
			std::vector<double> v06(pixel_arr, pixel_arr + 10);
			std_dev_num = std_dev_calc(v06);
			mean_num = std::accumulate(v06.begin(), v06.end(), 0.0) / v06.size();
			mean6.push_back(mean_num);
			variance6.push_back(std_dev_num);

			z = 0;
			for (i = 60; i < 70; ++i) {
				pixel_arr[z] = array3D[((k-1)*width + (j-1))+(i*width*height)];
				z++;
			}
			std::vector<double> v07(pixel_arr, pixel_arr + 10);
			std_dev_num = std_dev_calc(v07);
			mean_num = std::accumulate(v07.begin(), v07.end(), 0.0) / v07.size();
			mean7.push_back(mean_num);
			variance7.push_back(std_dev_num);

			z = 0;
			for (i = 70; i < 80; ++i) {
				pixel_arr[z] = array3D[((k-1)*width + (j-1))+(i*width*height)];
				z++;
			}
			std::vector<double> v08(pixel_arr, pixel_arr + 10);
			std_dev_num = std_dev_calc(v08);
			mean_num = std::accumulate(v08.begin(), v08.end(), 0.0) / v08.size();
			mean8.push_back(mean_num);
			variance8.push_back(std_dev_num);

			z = 0;
			for (i = 80; i < 90; ++i) {
				pixel_arr[z] = array3D[((k-1)*width + (j-1))+(i*width*height)];
				z++;
			}
			std::vector<double> v09(pixel_arr, pixel_arr + 10);
			std_dev_num = std_dev_calc(v09);
			mean_num = std::accumulate(v09.begin(), v09.end(), 0.0) / v09.size();
			mean9.push_back(mean_num);
			variance9.push_back(std_dev_num);
			cout << "v09 " << mean_num <<  '\t' << std_dev_num << endl;

/*
			z = 0;
			for (i = 90; i < 100; ++i) {
				pixel_arr[z] = array3D[((k-1)*width + (j-1))+(i*width*height)];
				z++;
			}

			std::vector<double> v10(pixel_arr, pixel_arr + 10);
			std_dev_num = std_dev_calc(v10);
			mean_num = std::accumulate(v10.begin(), v10.end(), 0.0) / v10.size();
			mean10.push_back(mean_num);
			variance10.push_back(std_dev_num);
			*/
		}
	}

	//Get median of all mean and variance sets, and put them into a vector

	std::vector <double> mean_all(0);
	std::vector <double> variance_all(0);
		
	mean_all.push_back(inPlaceMedian(mean1));
	variance_all.push_back(inPlaceMedian(variance1));

	mean_all.push_back(inPlaceMedian(mean2));
	variance_all.push_back(inPlaceMedian(variance2));

	mean_all.push_back(inPlaceMedian(mean3));
	variance_all.push_back(inPlaceMedian(variance3));

	mean_all.push_back(inPlaceMedian(mean4));
	variance_all.push_back(inPlaceMedian(variance4));

	mean_all.push_back(inPlaceMedian(mean5));
	variance_all.push_back(inPlaceMedian(variance5));

	mean_all.push_back(inPlaceMedian(mean6));
	variance_all.push_back(inPlaceMedian(variance6));

	mean_all.push_back(inPlaceMedian(mean7));
	variance_all.push_back(inPlaceMedian(variance7));

	mean_all.push_back(inPlaceMedian(mean8));
	variance_all.push_back(inPlaceMedian(variance8));

	mean_all.push_back(inPlaceMedian(mean9));
	variance_all.push_back(inPlaceMedian(variance9));

	for (i = 0; i < mean_all.size(); i++) {
		printf("%d\n",mean_all[i]);
	}

	//mean_all.push_back(inPlaceMedian(mean10));
	//variance_all.push_back(inPlaceMedian(variance10));

	// Use median mean and variance vectors to calculate the total gain and read noise

	double c0 = 0, c1 = 0, cov00 = 0, cov01 = 0, cov11 = 0, sumsq = 0;													// Calculate gain and read noise from a linear fit to the data
  	gsl_fit_linear(&mean_all[0], 1, &variance_all[0], 1, mean_all.size(), &c0, &c1, &cov00, &cov01, &cov11, &sumsq);
	std::cout << "M (Gain): " << 1/c1 << '\n' << "B (Read Noise): " << c0/c1 << std::endl;

	for (i = 0; i < c_countOfImagesToGrab; ++i) {
		fits_close_file(fpt_arr.at(i), &exitCode);
	}
	
	return exitCode;
}

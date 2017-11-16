/*! \file sigma_clip_mixed.cpp
\brief A documented file that sigma clips and finds the median of already made files.
*/

#include "write_basler_fits.h"
#include <numeric>
#include <cmath>

//  std_dev_calc function
/** Takes in a vector structure and finds the standard deviation of the data elements
* \return an integer
*/
double std_dev_calc(std::vector<double> v) {
	double mean = std::accumulate(v.begin(), v.end(), 0.0) / v.size();
	double std_dev = 0;
	for (int i = 0; i < v.size(); ++i) {
		//std::cout << v[i] << std::endl;
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
	int i, exp = 500;
	for (i = 0; i < c_countOfImagesToGrab; i++) {
		if (i % 100 == 0 && i > 0)
			exp = exp + 400;
		expArray[i] = exp;
	}
	const char *names[c_countOfImagesToGrab];
	for (i = 0; i < c_countOfImagesToGrab; ++i) {  //creates an array of names for each of the files previously made
		char filename[100];
		//strncpy(filename, "fitsimg_exp", sizeof(filename));
		strncpy(filename, "/home/cbohlman/Documents/caao_summer2017/ExAOLab/Bohlman/code/raw_data_110917/fitsimg_exp", sizeof(filename));
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
	std::vector<double> array3D(width*height*c_countOfImagesToGrab);

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
	
	
	for (k = 1; k <= height; k = k+10) {   //Looks through each pixel in a picture
		for (j = 1; j <= width; j = j+10) {
			//cout << k << '\t' << j << endl;
			double pixel_arr[1000];
			int z = 0;
			
			for (i = 0; i < 100; ++i) {
				pixel_arr[z] = array3D[((k-1)*width + (j-1))+(i*width*height)];
				//cout << ((k-1)*width + (j-1))+(i*width*height)-1 << endl;
				z++;
			}
			std::vector<double> v01(pixel_arr, pixel_arr + 100);
			double std_dev_num = std_dev_calc(v01);
			double mean_num = std::accumulate(v01.begin(), v01.end(), 0.0) / v01.size();
			cout << mean_num <<  '\t' << std_dev_num << endl;
			//cout << endl;
			z = 0;
			for (i = 100; i < 200; ++i) {
				pixel_arr[z] = array3D[((k-1)*width + (j-1))+(i*width*height)];
				z++;
			}
			std::vector<double> v02(pixel_arr, pixel_arr + 100);
			std_dev_num = std_dev_calc(v02);
			mean_num = std::accumulate(v02.begin(), v02.end(), 0.0) / v02.size();
			cout << mean_num <<  '\t' << std_dev_num << endl;
			//cout << endl;
			z = 0;
			for (i = 200; i < 300; ++i) {
				pixel_arr[z] = array3D[((k-1)*width + (j-1))+(i*width*height)];
				z++;
			}
			std::vector<double> v03(pixel_arr, pixel_arr + 100);
			std_dev_num = std_dev_calc(v03);
			mean_num = std::accumulate(v03.begin(), v03.end(), 0.0) / v03.size();
			cout << mean_num <<  '\t' << std_dev_num << endl;
			//cout << endl;
			z = 0;
			for (i = 300; i < 400; ++i) {
				pixel_arr[z] = array3D[((k-1)*width + (j-1))+(i*width*height)];
				z++;
			}
			std::vector<double> v04(pixel_arr, pixel_arr + 100);
			std_dev_num = std_dev_calc(v04);
			mean_num = std::accumulate(v04.begin(), v04.end(), 0.0) / v04.size();
			cout << mean_num <<  '\t' << std_dev_num << endl;
			//cout << endl;
			z = 0;
			for (i = 400; i < 500; ++i) {
				pixel_arr[z] = array3D[((k-1)*width + (j-1))+(i*width*height)];
				z++;
			}
			std::vector<double> v05(pixel_arr, pixel_arr + 100);
			std_dev_num = std_dev_calc(v05);
			mean_num = std::accumulate(v05.begin(), v05.end(), 0.0) / v05.size();
			cout << mean_num <<  '\t' << std_dev_num << endl;
			
			z = 0;
			for (i = 500; i < 600; ++i) {
				pixel_arr[z] = array3D[((k-1)*width + (j-1))+(i*width*height)];
				z++;
			}
			std::vector<double> v06(pixel_arr, pixel_arr + 100);
			std_dev_num = std_dev_calc(v06);
			mean_num = std::accumulate(v06.begin(), v06.end(), 0.0) / v06.size();
			cout << mean_num <<  '\t' << std_dev_num << endl;
			//cout << endl;
			z = 0;
			for (i = 600; i < 700; ++i) {
				pixel_arr[z] = array3D[((k-1)*width + (j-1))+(i*width*height)];
				z++;
			}
			std::vector<double> v07(pixel_arr, pixel_arr + 100);
			std_dev_num = std_dev_calc(v07);
			mean_num = std::accumulate(v07.begin(), v07.end(), 0.0) / v07.size();
			cout << mean_num <<  '\t' << std_dev_num << endl;
			//cout << endl;
			z = 0;
			for (i = 700; i < 800; ++i) {
				pixel_arr[z] = array3D[((k-1)*width + (j-1))+(i*width*height)];
				z++;
			}
			std::vector<double> v08(pixel_arr, pixel_arr + 100);
			std_dev_num = std_dev_calc(v08);
			mean_num = std::accumulate(v08.begin(), v08.end(), 0.0) / v08.size();
			cout << mean_num <<  '\t' << std_dev_num << endl;
			//cout << endl;
			z = 0;
			for (i = 800; i < 900; ++i) {
				pixel_arr[z] = array3D[((k-1)*width + (j-1))+(i*width*height)];
				z++;
			}
			std::vector<double> v09(pixel_arr, pixel_arr + 100);
			std_dev_num = std_dev_calc(v09);
			mean_num = std::accumulate(v09.begin(), v09.end(), 0.0) / v09.size();
			cout << mean_num <<  '\t' << std_dev_num << endl;
			//cout << endl;
			z = 0;
			for (i = 900; i < 1000; ++i) {
				pixel_arr[z] = array3D[((k-1)*width + (j-1))+(i*width*height)];
				z++;
			}
			std::vector<double> v10(pixel_arr, pixel_arr + 100);
			std_dev_num = std_dev_calc(v10);
			mean_num = std::accumulate(v10.begin(), v10.end(), 0.0) / v10.size();
			cout << mean_num <<  '\t' << std_dev_num << endl;
			//cout << endl;
		}
	}
	
	for (i = 0; i < c_countOfImagesToGrab; ++i) {
		fits_close_file(fpt_arr.at(i), &exitCode);
	}
	
	return exitCode;
}

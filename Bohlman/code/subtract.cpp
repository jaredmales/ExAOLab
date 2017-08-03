#include "stdafx.h"
#include "write_basler_fits.h"

int main(void) {
	std::ios_base::sync_with_stdio(false);  	// Seperates std from stdio
	int exitCode = 0;
	char file1[25];
	char file2[25];
	scanf("%s", file1);
	scanf("%s", file2);
	fitsfile *fptr1, *fptr2;
	if (fits_open_file(&fptr1, file1, READONLY, &exitCode)) {
		fits_report_error(stderr, exitCode);  // Prints out any fits error messages
		return 1;
	}

	if (fits_open_file(&fptr2, file2, READONLY, &exitCode)) {
		fits_report_error(stderr, exitCode);  // Prints out any fits error messages
		return 1;
	}

	long fpixel[2] = { 1,1 };
	double * pixel_arr1, *pixel_arr2, *new_arr;
	pixel_arr1 = new double[640 * 480];
	pixel_arr2 = new double[640 * 480];
	new_arr = new double[640 * 480];
	int nelements = 640 * 480;

	if (fits_read_pix(fptr1, TDOUBLE, fpixel, nelements, NULL, pixel_arr1, NULL, &exitCode)) {
		fits_report_error(stderr, exitCode);  // Prints out any fits error messages
		return 1;
	}

	if (fits_read_pix(fptr2, TDOUBLE, fpixel, nelements, NULL, pixel_arr2, NULL, &exitCode)) {
		fits_report_error(stderr, exitCode);  // Prints out any fits error messages
		return 1;
	}

	int j, k;
	int width = 640, height = 480;
	for (k = 0; k < height; ++k) {   //Subract arrays to new array
		for (j = 0; j < width; ++j) {
			new_arr[k*width + j] = pixel_arr1[k*width + j] - pixel_arr2[k*width + j];
		}
	}
	long fpixel2[2] = { 1,1 };
	long naxes[2] = { width, height };
	long  fpixel3 = 1;
	long naxis = 2;


	fitsfile *fptr3;
	if (fits_create_file(&fptr3, "!output.fits", &exitCode) != 0) //Creates new fits file
	{
		fits_report_error(stderr, exitCode);  // Prints out any fits error messages
		return 1;
	}
	if (fits_create_img(fptr3, DOUBLE_IMG, naxis, naxes, &exitCode) != 0)  //Creates the primary array image
	{
		fits_report_error(stderr, exitCode);  // Prints out any fits error messages
		return 1;
	}
	if (fits_write_img(fptr3, TDOUBLE, fpixel3, width*height, new_arr, &exitCode) != 0)  // Writes pointer values to the image
	{
		fits_report_error(stderr, exitCode);  // Prints out any fits error messages
		return 1;
	}
	if (fits_close_file(fptr3, &exitCode) != 0) // Closes the fits file
	{
		fits_report_error(stderr, exitCode);  // Prints out any fits error messages
		return 1;
	}

	fits_close_file(fptr1, &exitCode);
	fits_close_file(fptr2, &exitCode);
	delete(pixel_arr1);
	delete(pixel_arr2);
	delete(new_arr);

	cout << endl << "Press Enter to exit." << endl;
	while (cin.get() != '\n');

	return exitCode;
}

/*Fixes to the problem of the program taking forever:
Don't open every file 640*480 times, have an array of fits pointers
Put values directly into vector

/*! \file gain_read_blank.cpp
\brief A documented file that
Initializes the pylon resources, takes the photos, finds median photo.
*/

#include "stdafx.h"
#include "write_basler_fits.h"

//  Gets median image of the previous images.   
/** Gets the names of the images, and goes into a for loop which gets the median of each pixel for each image and applies it to an int array, which then writes to a fits file
*  \return an integer: 0 upon exit success, 1 otherwise
*/


//Assumes both parameters both point to fits files that have already been opened
int subtract_images(char* file_name){
	int exitCode = 0;
	const char* median_name = "median_lowexp_image.fits";
	fitsfile *fptr1, *fptr2;
	if (fits_open_file(&fptr1, file_name, READWRITE, &exitCode))
		fits_report_error(stderr, exitCode);  // Prints out any fits error messages

	if (fits_open_file(&fptr2, median_name, READONLY, &exitCode))
		fits_report_error(stderr, exitCode);  // Prints out any fits error messages

	long fpixel[2] = { 1,1 };
	double * pixel_arr1, * pixel_arr2, * new_arr;
	pixel_arr1 = new double[640 * 480];
	pixel_arr2 = new double[640 * 480];
	new_arr = new double[640 * 480];
	int nelements = 640 * 480;

	if (fits_read_pix(fptr1, TDOUBLE, fpixel, nelements, NULL, pixel_arr1, NULL, &exitCode))
		fits_report_error(stderr, exitCode);  // Prints out any fits error messages

	if (fits_read_pix(fptr2, TLONGLONG, fpixel, nelements, NULL, pixel_arr2, NULL, &exitCode))
		fits_report_error(stderr, exitCode);  // Prints out any fits error messages

	int j, k;
	int width = 640, height = 480;
	for (k = 0; k < height; ++k) {   //Subract arrays to new array
		for (j = 0; j < width; ++j) {
			new_arr[k*width + j] = pixel_arr1[k*width + j] - pixel_arr2[k*width + j];
		}
	}
	long fpixel2[2] = { 1,1 };
	if (fits_write_pix(fptr1, TDOUBLE, fpixel2, nelements, new_arr, &exitCode) != 0)  // Writes pointer values to the image
		fits_report_error(stderr, exitCode);  // Prints out any fits error messages

	fits_close_file(fptr1, &exitCode);
	fits_close_file(fptr2, &exitCode);
	delete(pixel_arr1);
	delete(pixel_arr2);
	delete(new_arr);

	return exitCode;
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
	i = 0;
	std::ios_base::sync_with_stdio(false);  	// Seperates std from stdio
	PylonInitialize();  	// Initializes pylon runtime before using any pylon methods
	try
	{
		CBaslerUsbInstantCamera camera(CTlFactory::GetInstance().CreateFirstDevice());   // Creates an instant camera object with the camera device found first.
		string file_name = (string)camera.GetDeviceInfo().GetModelName() + " " + (string)camera.GetDeviceInfo().GetSerialNumber();
		char* newstr = &file_name[0u];
		camera.StartGrabbing(c_countOfImagesToGrab);  		// Starts the grabbing of c_countOfImagesToGrab images.
		CGrabResultPtr ptrGrabResult;
		while (camera.IsGrabbing())
		{
			int exposure = expArray[i];  // Gets the desired exposure time from function get_exposure() 
			camera.Basler_UsbCameraParams::CUsbCameraParams_Params::ExposureTime.SetValue(exposure);   // Sets the exposure of the next camera shot.
			int tempcam = (int)camera.Basler_UsbCameraParams::CUsbCameraParams_Params::DeviceTemperature.GetValue();
			camera.RetrieveResult(5000, ptrGrabResult, TimeoutHandling_ThrowException);  // Waits for an image and then retrieves it. A timeout of 5000 ms is used.
			if (ptrGrabResult->GrabSucceeded())  // If image is grabbed successfully: 
			{
				struct image *cam_image = new struct image; //Setting up image struct

				char real_filename[30];
				strncpy(real_filename, "!", sizeof(real_filename));
				strcat(real_filename, "fitsimg_exp");
				char exp_str[10];
				sprintf(exp_str, "%d", exposure);
				strcat(real_filename, exp_str);
				char num_str[10];
				sprintf(num_str, "_%d", i);
				strcat(real_filename, num_str);
				strcat(real_filename, ".fits");

				cam_image->imgname = real_filename;
				cam_image->imgGrab = ptrGrabResult;
				cam_image->exposure = exposure;
				cam_image->temp = tempcam;
				cam_image->camname = _strdup(newstr);

				if (write_basler_fits(cam_image) != 0)  //if image building did not work
				{
					throw "Bad process in fits image writing!";
					free(cam_image->camname);
					delete(cam_image);
				}
				else {									//if image building did work
					cout << "Image grab and write successful" << endl;
					*(cam_image->imgname)++;
					if (subtract_images(cam_image->imgname) == 0) {
						cout << "Median image subtracted" << endl;
					}
					free(cam_image->camname);
					delete(cam_image);
				}
			}
			else  // If image is not grabbed successfully
			{
				cout << "Error: " << ptrGrabResult->GetErrorCode() << " " << ptrGrabResult->GetErrorDescription() << endl; //Prints an error
			}
			++i;
		}
	}
	catch (const GenericException &e)  // Provides error handling.
	{
		std::cerr << "An exception occurred." << endl
			<< e.GetDescription() << endl;
		exitCode = 1;
	}

	std::cerr << endl << "Press Enter to exit." << endl;
	while (cin.get() != '\n');
	Pylon::PylonTerminate();   // Releases all pylon resources. 
	return exitCode;
}

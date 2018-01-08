/*! \file gain_read_mixed_exp.cpp
\brief A documented file that takes 100 different pictures for 10 exposure times, for a total of 1000 images taken. Can subtract median photo from each.
Initializes the pylon resources, takes each photo and subtracts the median photo from each.
*/

#include "write_basler_fits.h"

//  Subtracts a median image from the given file name
/** Gets the name of the file, opens it, opens the median file, subtracts the median file from the given file name, writes the new file.
*  \return an integer: 0 upon exit success, 1 otherwise
*/
int subtract_images(char* file_name){
	int exitCode = 0;
	const char* median_name = "median_lowexp_image.fits";											// Name of the median image that is subtracted from each image
	fitsfile *fptr1, *fptr2;
	if (fits_open_file(&fptr1, file_name, READWRITE, &exitCode)) {									// Open image file passed in function			
		fits_report_error(stderr, exitCode);  														// if it does not exist, print out any fits error messages
		return 1;
	}

	if (fits_open_file(&fptr2, median_name, READONLY, &exitCode)) {									// Open median image file
		fits_report_error(stderr, exitCode);  														// If it does not exist, print out any fits error messages
		return 1;
	}

	long fpixel[2] = { 1,1 };																		// Instructs cfitsio to read a pixel at a time
	double * pixel_arr1, * pixel_arr2, * new_arr;
	pixel_arr1 = new double[640 * 480];																// Array that holds first image data
	pixel_arr2 = new double[640 * 480];																// Array that holds second image data
	new_arr = new double[640 * 480];																// Array that holds final image data
	int nelements = 640 * 480;

	if (fits_read_pix(fptr1, TDOUBLE, fpixel, nelements, NULL, pixel_arr1, NULL, &exitCode)) {		// Store image data into array
		fits_report_error(stderr, exitCode);  														// Prints out any fits error messages
		return 1;
	}

	if (fits_read_pix(fptr2, TLONGLONG, fpixel, nelements, NULL, pixel_arr2, NULL, &exitCode)) {	// Store image data into array
		fits_report_error(stderr, exitCode);  														// Prints out any fits error messages
		return 1;
	}

	int j, k;																						// Subtract median image from passed file and store value into new array
	int width = 640, height = 480;
	for (k = 0; k < height; ++k) {
		for (j = 0; j < width; ++j) {
			new_arr[k*width + j] = pixel_arr1[k*width + j] - pixel_arr2[k*width + j];
		}
	}
	long fpixel2[2] = { 1,1 };
	if (fits_write_pix(fptr1, TDOUBLE, fpixel2, nelements, new_arr, &exitCode) != 0) {  			// Writes pointer values to the image
		fits_report_error(stderr, exitCode);  														// Prints out any fits error messages
		return 1;
	}

	fits_close_file(fptr1, &exitCode);																// Close and free up everything
	fits_close_file(fptr2, &exitCode);
	delete(pixel_arr1);
	delete(pixel_arr2);
	delete(new_arr);

	return exitCode;
}

//  Main function
/** Initializes pylon resources, takes pictures, subtracts median image, closes all pylon resources.
* \return an integer: 0 upon exit success, 1 otherwise
*/
int main(int argc, ///< [in] the integer value of the count of the command line arguments
	char* argv[] ///< [ch.ar] the integer value of the count of the command line arguments
)
{
	int exitCode = 0;
	int expArray[c_countOfImagesToGrab];																								// Create an array of exposure values
	int i, exp = 500;																													// 500, 900, 1300, 1800, 2200, 2600, 3000, 3400, 3800, 4200
	for (i = 0; i < c_countOfImagesToGrab; i++) {
		if (i % 100 == 0 && i > 0)
			exp = exp + 400;
		expArray[i] = exp;
	}
	i = 0;							
	PylonInitialize();  																												// Initializes pylon runtime before using any pylon methods
	try
	{
		CDeviceInfo info;																												// Variable for camera info
		info.SetDeviceClass(Camera_t::DeviceClass());																					// Get camera info
		Camera_t camera(CTlFactory::GetInstance().CreateFirstDevice());   																// Creates an instant camera object with the camera device found first.
		
		for (int j = 0; j < c_countOfImagesToGrab; ++j)																					// For every image
		{
			string file_name = (string)camera.GetDeviceInfo().GetModelName() + " " + (string)camera.GetDeviceInfo().GetSerialNumber();	// Get camera name from model number and serial number
			char* newstr = &file_name[0u];																								// Cast camera name to char* because of cfitsio
			CGrabResultPtr ptrGrabResult;
			int exposure = expArray[j];  																								// Get desired exposure value from array 

			camera.Open();																												// Open camera parameters
			camera.ExposureAuto.SetValue(ExposureAuto_Off);																				// Turn off auto exposure
			camera.ExposureTime.SetValue(exposure);																						// Set exposure to desired value

			camera.StartGrabbing(1);  																									// Starts the grabbing of c_countOfImagesToGrab images.
			int tempcam = (int)camera.Basler_UsbCameraParams::CUsbCameraParams_Params::DeviceTemperature.GetValue();					// Get camera temperature
			camera.RetrieveResult(5000, ptrGrabResult, TimeoutHandling_ThrowException);  												// Waits for an image and then retrieves it. A timeout of 5000 ms is used.
			camera.Close();																												// Close camera parameters
			if (ptrGrabResult->GrabSucceeded())  																						// If image is grabbed successfully: 
			{		
				struct image *cam_image = new struct image; 																			// Set up image struct

				char real_filename[30];																									// Set up image file name from given strings, exposure value, and image number
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
				cam_image->camname = newstr;

				if (write_basler_fits(cam_image) != 0)  																				// if image building did not work
				{
					throw "Bad process in fits image writing!";																			// throw error
					exitCode = 1;
					delete(cam_image);																									// free struct
				}
				else {																													// if image building did work
					cout << "Image grab and write successful" << endl;
/*
					*(cam_image->imgname)++;																							// Subtract image
					if (subtract_images(cam_image->imgname) == 0) {
						cout << "Median image subtracted" << endl;
					}
					else {
						cout << "Error in file subtracting process" << endl;
						exitCode = 1;
					}
*/
					delete(cam_image);																									// free struct
				}
			}
			else  																														// If image is not grabbed successfully
			{
				cout << "Error: " << ptrGrabResult->GetErrorCode() << " " << ptrGrabResult->GetErrorDescription() << endl; 				//Prints an error
			}
			++i;
		}
	}
	catch (const GenericException &e)  																									// Provides error handling.
	{
		std::cerr << "An exception occurred." << endl
			<< e.GetDescription() << endl;
		exitCode = 1;
	}
	Pylon::PylonTerminate();   																											// Releases all pylon resources. 
	return exitCode;
}

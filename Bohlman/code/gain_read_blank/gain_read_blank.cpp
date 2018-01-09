/*! \file gain_read_blank.cpp
\brief A documented file that initializes the pylon resources, takes a number of images at the lowest exposure, and finds/writes median image in rder to make an averaged flat image
*/

#include "write_basler_fits.h"

//  Gets median image of the previous images.   
/** Gets the names of the images, and goes into a for loop which gets the median of each pixel for each image and applies it to an int array, which then writes to a fits file
*  \return an integer: 0 upon exit success, 1 otherwise
*/
int find_median()
{
	int exitCode = 0;
	const char *names[c_countOfImagesToGrab];														// Creates an array of filenames to read
	int kk = 0;
	for (kk; kk < c_countOfImagesToGrab; ++kk) 
	{  													
		char base_filename[30];
		strncpy(base_filename, "fitsimg_exp59_", sizeof(base_filename));
		char num[3];
		sprintf(num, "%d", kk);
		strcat(base_filename, num);
		strcat(base_filename, ".fits");
		names[kk] = base_filename;
	}

	std::vector<fitsfile*> fpt_arr(c_countOfImagesToGrab);											// Creates array of fits pointers from file names and opens all of them
	int cc;
	for (cc = 0; cc < c_countOfImagesToGrab; ++cc) 
	{
		fitsfile* fptr;
		fpt_arr.push_back(fptr);
		fits_open_file(&(fpt_arr.at(cc)), names[cc], READONLY, &exitCode);
	}

	int width = 640, height = 480;
	double *image_arr = (double*)calloc(width * height, sizeof(double)); 							// Creates array of individual pixel values for final image
	int j, k;
	for (k = 1; k <= height; ++k) 
	{  																// Looks through each pixel sequentially
		for (j = 1; j <= width; ++j) 
		{
			int ii;
			double pixel_arr[c_countOfImagesToGrab];
			for (ii = 0; ii < c_countOfImagesToGrab; ++ii) { 										// Look at the pixel location for each image
				long fpixel[2] = { j,k };
				double pixels;
				fits_read_pix(fpt_arr.at(ii), TDOUBLE, fpixel, 1, NULL, &pixels, NULL, &exitCode);	// Read a singular pixel value at location
				pixel_arr[ii] = pixels;																
			}1
			std::vector<double> v(pixel_arr, pixel_arr + c_countOfImagesToGrab);  					// Find median of all pixel values at specific location and add to final image array at that location
			std::nth_element(v.begin(), v.begin() + v.size() / 2, v.end());
			image_arr[(k - 1)*width + (j - 1)] = v[v.size() / 2];
		}
		if (k % 40 == 0) 																			// Displays progress
			cout << "Processing images..." << endl;
	}

	for (cc = 0; cc < c_countOfImagesToGrab; ++cc) {												// Close all fits files
		fits_close_file(fpt_arr.at(cc), &exitCode);
	}

	fitsfile *fptr;																					// Create a fits file for the median image, check for errors throughout creation process
	long naxes[2] = { width, height };
	long naxis = 2, fpixel = 1;

	if (fits_create_file(&fptr, "!median_lowexp_image.fits", &exitCode) != 0) {
		exitCode = 1; 						
		fits_report_error(stderr, exitCode);
	}  														

	if (fits_create_img(fptr, LONGLONG_IMG, naxis, naxes, &exitCode) != 0) { 			
		exitCode = 1;			
		fits_report_error(stderr, exitCode);  														
	}

	if (fits_write_img(fptr, TDOUBLE, fpixel, width*height, image_arr, &exitCode) != 0) { 	
		exitCode = 1;		
		fits_report_error(stderr, exitCode);  	
	}													

	if (fits_close_file(fptr, &exitCode) != 0) {		
		exitCode = 1;											
		fits_report_error(stderr, exitCode);
	}  														

	free(image_arr);
	std::cout << "Median image at minimum exposure produced!" << endl; 								// Completion message
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
	int expArray[c_countOfImagesToGrab];																									// Sets up array of exposures (in this case, all lowest possible value)
	for (int i = 0; i < c_countOfImagesToGrab; i++)
		expArray[i] = 59;
	PylonInitialize();  																													// Initializes pylon runtime before using any pylon methods
	try
	{
		CDeviceInfo info;																													// Set up attached Basler USB camera
		info.SetDeviceClass(Camera_t::DeviceClass());
		Camera_t camera(CTlFactory::GetInstance().CreateFirstDevice());   																	// Creates an instant camera object with the camera device found first.
		for (int j = 0; j < c_countOfImagesToGrab; ++j)																						// For the desired amount of images
		{
			string file_name = (string)camera.GetDeviceInfo().GetModelName() + " " + (string)camera.GetDeviceInfo().GetSerialNumber();		// Gets camera name from model name and serial number
			char* newstr = &file_name[0u];																									// Stores camera name into a char* for cfitsio
			CGrabResultPtr ptrGrabResult;															
			int exposure = expArray[j];  																									// Gets the desired exposure time from the exposure array 
			
			camera.Open();																													// Opens camera parameters
			camera.ExposureAuto.SetValue(ExposureAuto_Off);
			camera.ExposureTime.SetValue(exposure);
			
			camera.StartGrabbing(1);  																										// Starts the grabbing of a singular image.
			int tempcam = (int)camera.Basler_UsbCameraParams::CUsbCameraParams_Params::DeviceTemperature.GetValue();						//Gets camera temperature
			camera.RetrieveResult(5000, ptrGrabResult, TimeoutHandling_ThrowException);  													// Waits for an image and then retrieves it. A timeout of 5000 ms is used.
			camera.Close();
			if (ptrGrabResult->GrabSucceeded())  																							// If image is grabbed successfully: 
			{
				struct image *cam_image = new struct image; 																				// Set up image struct
				cam_image->imgGrab = ptrGrabResult;
				cam_image->exposure = exposure;
				cam_image->temp = tempcam;
				cam_image->camname = newstr;

				char real_filename[25];																										// Set up image file name with given strings, exposure time, and image number
				strncpy(real_filename, "!", sizeof(real_filename));
				strcat(real_filename, "fitsimg_exp");
				char exp_str[6];
				sprintf(exp_str, "%d", cam_image->exposure);
				strcat(real_filename, exp_str);
				char num_str[6];
				sprintf(num_str, "_%d", j);
				strcat(real_filename, num_str);
				strcat(real_filename, ".fits");
				cam_image->imgname = real_filename;

				if (write_basler_fits(cam_image) != 0)  																					// If image building did not work
				{
					throw "Bad process in fits image writing!";																				// Throw an error
					delete(cam_image);																										// Free struct
				}
				else {																														// If image building did work
					cout << "Image grab and write successful" << endl;																		// Print confirmation message
					delete(cam_image);																										// Free struct
				}
			}
			else  																															// If image is not grabbed successfully, throw an error
			{
				cerr << "Error: " << ptrGrabResult->GetErrorCode() << " " << ptrGrabResult->GetErrorDescription() << endl;
			}
		}
		exitCode = find_median();																											// Find median of all images through find_median()
	}
	catch (const GenericException &e)  																										// Provides Pylon error handling.
	{
		std::cerr << "An exception occurred." << endl
			<< e.GetDescription() << endl;
		exitCode = 1;
	}
	Pylon::PylonTerminate();   																												// Releases all pylon resources. 
	return exitCode;
}

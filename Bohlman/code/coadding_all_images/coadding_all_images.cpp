/*! \file cycle_exposures.cpp
\brief A documented file that takes command line arguments for exposure and number of images to grab and outputs fits files
Initializes the pylon resources, takes the photos, and passes a struct to write_basler_fits() function with image data in it
*/

#include "write_basler_fits.h"
//  Subtracts a median image from the given file name
/** Gets the name of the file, opens it, opens the median file, subtracts the median file from the given file name, writes the new file.
*  \return an integer: 0 upon exit success, 1 otherwise
*/

//  Main function
/** Initializes pylon resources, takes pictures, closes all pylon resources.
* \return an integer: 0 upon exit success, 1 otherwise
*/
int main(int argc, ///< [in] the integer value of the count of the command line arguments
	char* argv[] ///< [ch.ar] the integer value of the count of the command line arguments
)
{
	int c_countOfImagesToGrab, z = 0;
	uint32_t countOfImagesToGrab;
	int exitCode = 0;
	int exposure = 0, tempcam;
	if (argc == 3) {																															// if there are more than 2 command line arguments
		exposure = atoi(argv[1]);																														// exposure is first argument																														
		countOfImagesToGrab = atoi(argv[2]);																									// number of images to grab is second argument
	}
	else {																																	// if there are less than 2 command line arguments
		exposure = 1000;																															// default value of exp: 5000 us
		countOfImagesToGrab = 100;																											// default number of images to grab: 10
	}			
	PylonInitialize();  																													// Initializes pylon runtime before using any pylon methods
	try
	{
		CDeviceInfo info;																													// Set up attached Basler USB camera
		info.SetDeviceClass(Camera_t::DeviceClass());
		CBaslerUsbInstantCamera camera( CTlFactory::GetInstance().CreateFirstDevice());
		string file_name = (string)camera.GetDeviceInfo().GetModelName() + " " + (string)camera.GetDeviceInfo().GetSerialNumber();		// Gets camera model name and serial number
		char* newstr = &file_name[0u];																									// Casts string as char* for cfitsio
		camera.Open();																														// Opens camera parameters to grab images and set exposure time
		camera.ExposureAuto.SetValue(ExposureAuto_Off);																						// Set exposure
		camera.ExposureTime.SetValue(exposure);
		camera.PixelFormat.SetValue(PixelFormat_Mono10);
		int camera_iteration = 1;
		int nelements = 640 * 480;
		int j, k;																						// Subtract median image from passed file and store value into new array
		int width = 640, height = 480;
		unsigned int *new_arr = new unsigned int[nelements]();																// Array that holds final image data

		camera.StartGrabbing(countOfImagesToGrab);																							// Start grabbing a provided amount of images
		while (camera.IsGrabbing()) {

			CGrabResultPtr ptrGrabResult;
			tempcam = (int)camera.DeviceTemperature.GetValue();																			// Gets and stores temperature of camera
			camera.RetrieveResult(5000, ptrGrabResult, TimeoutHandling_ThrowException);  													// Waits for an image and then retrieves it. A timeout of 5000 ms is used
			if (ptrGrabResult->GrabSucceeded())  																							// If image is grabbed successfully 
			{
				uint16_t *pImageBuffer = (uint16_t *)ptrGrabResult->GetBuffer();
				
				for (k = 0; k < height; ++k) 
				{
					for (j = 0; j < width; ++j) 
					{
						new_arr[k*width + j] = new_arr[k*width + j] + pImageBuffer[k*width + j];
						//cout << j << '\t' << k << endl;
					}
					//cout << new_arr[k*width + j] << endl;	
				}		
			}
			else  																															// If image is not grabbed successfully, throw an error
			{
				cerr << "Error: " << ptrGrabResult->GetErrorCode() << " " << ptrGrabResult->GetErrorDescription() << endl;
				exitCode = 1;
			}
		}
		for (k = 0; k < height; ++k) 
		{
			for (j = 0; j < width; ++j) 
			{
				new_arr[k*width + j] = new_arr[k*width + j] / countOfImagesToGrab;
			}
		}

		struct image *cam_image = new struct image; 																				// Setting up image struct
		cam_image->imgGrab = new_arr;
		cam_image->exposure = exposure;
		cam_image->temp = tempcam;
		cam_image->camname = newstr;
		char real_filename[40];																										// Construct file name from given strings, exposure time, and image number
		strncpy(real_filename, "!", sizeof(real_filename));
		strcat(real_filename, "fitsimg_coadded_exp");
		char exp_str[6];
		sprintf(exp_str, "%d", cam_image->exposure);
		strcat(real_filename, exp_str);
		char num_str[6];
		sprintf(num_str, "_%d", z);
		strcat(real_filename, num_str);
		strcat(real_filename, ".fits");
		cam_image->imgname = real_filename;

		if (write_basler_fits(cam_image) != 0)  																					// If image building from struct did not work
		{
			throw "Bad process in fits image writing!";																				// Throws error
		}
		else 																														// If image building from struct did work
		{																															
			cout << "Image grab and write successful" << endl;																		// print confirmation message																				
		}
		delete(cam_image);																											// Free struct

		camera.Close();	
	}
	catch (const GenericException &e)  																										// Provides Basler error handling.
	{
		cerr << "An exception occurred." << endl
			<< e.GetDescription() << endl;
		exitCode = 1;
	}
	PylonTerminate();   																													// Releases all pylon resources. 
	return exitCode;
}

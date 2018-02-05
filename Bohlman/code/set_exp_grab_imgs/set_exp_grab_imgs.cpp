/*! \file cycle_exposures.cpp
\brief A documented file that takes and writes an image for 10 different exposures
Initializes the pylon resources, takes the photos, and passes a struct to write_basler_fits() function with image data in it
*/

#include "write_basler_fits.h"

//  Main function
/** Initializes pylon resources, takes pictures, closes all pylon resources.
* \return an integer: 0 upon exit success, 1 otherwise
*/

int get_exposure() 
{
	int exposure;  																													// Holds exposure time from stdin
	cin >> exposure;																												// Input desired exposure time
	//std::cin.ignore(std::cin.rdbuf()->in_avail());
	return exposure;  																												// Returns exposure time
}

int main(int argc, ///< [in] the integer value of the count of the command line arguments
	char* argv[] ///< [ch.ar] the integer value of the count of the command line arguments
)
{
	int exitCode = 0;
	int exp = get_exposure(); 																												// Define exposure here					
	PylonInitialize();  																													// Initializes pylon runtime before using any pylon methods
	try
	{
		CDeviceInfo info;																													// Set up attached Basler USB camera
		info.SetDeviceClass(Camera_t::DeviceClass());																						// Provides information about camera
		Camera_t camera(CTlFactory::GetInstance().CreateFirstDevice());   																	// Creates an instant camera object with the camera device found first
		for (int j = 0; j < c_countOfImagesToGrab; ++j)																										// Iterate through each exposure time
		{
			CGrabResultPtr ptrGrabResult;
			string file_name = (string)camera.GetDeviceInfo().GetModelName() + " " + (string)camera.GetDeviceInfo().GetSerialNumber();		// Gets camera model name and serial number
			char* newstr = &file_name[0u];																									// Casts string as char* for cfitsio
			int exposure = exp;  																									// Gets the desired exposure time from farray 

			camera.Open();																													// Opens camera parameters
			camera.ExposureAuto.SetValue(ExposureAuto_Off);																					// Turns off auto exposure
			camera.ExposureTime.SetValue(exposure);																							// Sets up exposure time from above value

			camera.StartGrabbing(1);  																										// Starts the grabbing of a singular image
			int tempcam = (int)camera.DeviceTemperature.GetValue();																			// Gets and stores temperature of camera
			camera.RetrieveResult(5000, ptrGrabResult, TimeoutHandling_ThrowException);  													// Waits for an image and then retrieves it. A timeout of 5000 ms is used
			camera.Close();																													// Closes camera parameters
			if (ptrGrabResult->GrabSucceeded())  																							// If image is grabbed successfully 
			{
				struct image *cam_image = new struct image; 																				// Setting up image struct
				cam_image->imgGrab = ptrGrabResult;
				cam_image->exposure = exposure;
				cam_image->temp = tempcam;
				cam_image->camname = newstr;
				char real_filename[25];																										// Construct file name from given strings, exposure time, and image number
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

				if (write_basler_fits(cam_image) != 0)  																					// If image building from struct did not work
				{
					throw "Bad process in fits image writing!";																				// Throws error
				}
				else 																														// If image building from struct did work
				{																															
					cout << "Image grab and write successful" << endl;																		// print confirmation message																				
				}
				delete(cam_image);																											// Free struct
			}
			else  																															// If image is not grabbed successfully, throw an error
			{
				cerr << "Error: " << ptrGrabResult->GetErrorCode() << " " << ptrGrabResult->GetErrorDescription() << endl;
				exitCode = 1;
			}
		}
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

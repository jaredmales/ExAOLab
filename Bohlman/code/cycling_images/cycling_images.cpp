/*! \file cycling_images.cpp
\brief A documented file that takes and writes 10 images indefinitely for a single exposure. Terminated with SIGINT (ctrl+c)
*/

#include "write_basler_fits.h"
#include <csignal>
#include <time.h>

/*
void signalHandler( int signum ) {
   cout << "Interrupt signal (" << signum << ") received.\n";

   // cleanup and close up stuff here  
   // terminate program  
   PylonTerminate();
   exit(signum);  
}
*/
//  Main function
/** Initializes pylon resources, takes pictures, closes all pylon resources.
* \return an integer: 0 upon exit success, 1 otherwise
*/
int main(int argc, ///< [in] the integer value of the count of the command line arguments
	char* argv[] ///< [ch.ar] the integer value of the count of the command line arguments
)
{
	//signal(SIGINT, signalHandler);																									// For signal handler
	struct timespec tim, tim2;																											// For time purposes
  	tim.tv_sec  = 1;
	tim.tv_nsec = 0;
	int exitCode = 0;
	int i = 0;																															// Keeps count of images
	int exposure = 5000;																												// Desired exposure time
	PylonInitialize();  																												// Initializes pylon runtime before using any pylon methods
	try
	{
		CDeviceInfo info;																												// Set up attached Basler USB camera
		info.SetDeviceClass(Camera_t::DeviceClass());
		Camera_t camera(CTlFactory::GetInstance().CreateFirstDevice());   																// Creates an instant camera object with the camera device found first
		CGrabResultPtr ptrGrabResult;
		string file_name = (string)camera.GetDeviceInfo().GetModelName() + " " + (string)camera.GetDeviceInfo().GetSerialNumber();		// Gets camera model and serial number
		char* newstr = &file_name[0u];																									// Casts camera name to a char* for cfitsio

		camera.Open();																													// Opens camera parameters
		camera.ExposureAuto.SetValue(ExposureAuto_Off);																					// Turns off camera auto exposure
		camera.ExposureTime.SetValue(exposure);																							// Sets exposure to above time
		camera.Close();
		camera.StartGrabbing();  																										// Start grabbing photos
		while (1) {																														// Infinite loop
			if (i == 10) i = 0;																											// For a cycle of 10 images
			int tempcam = (int)camera.Basler_UsbCameraParams::CUsbCameraParams_Params::DeviceTemperature.GetValue();					// Stores temperature of camera
			camera.RetrieveResult(5000, ptrGrabResult, TimeoutHandling_ThrowException);  												// Waits for an image and then retrieves it. A timeout of 5000 ms is used
			nanosleep(&tim , &tim2);																									// Rest for a second
			if (ptrGrabResult->GrabSucceeded())  																						// If image is grabbed successfully 
			{
				struct image *cam_image = new struct image; 																			// Set up image struct
				cam_image->imgGrab = ptrGrabResult;
				cam_image->exposure = exposure;
				cam_image->temp = tempcam;
				cam_image->camname = newstr;
				char real_filename[25];																									// Construct file name and store it in the struct
				strncpy(real_filename, "!", sizeof(real_filename));
				strcat(real_filename, "fitsimg");
				char exp_str[6];
				char num_str[6];
				sprintf(num_str, "_%d", i);
				strcat(real_filename, num_str);
				strcat(real_filename, ".fits");
				cam_image->imgname = real_filename;

				if (write_basler_fits(cam_image) != 0)  																				// if image building from struct did not work
				{
					throw "Bad process in fits image writing!";																			// throw error
					delete(cam_image);																									// Free struct
				}
				else {																													// if image building from struct did work
					cout << "Image grab and write successful" << endl;																	// Print confirmatio message
					delete(cam_image);																									// Free struct
				}
				i++;																													// Increment counter
			}
			else  																														// If image is not grabbed successfully, throw an error
			{
				cerr << "Error: " << ptrGrabResult->GetErrorCode() << " " << ptrGrabResult->GetErrorDescription() << endl;
				exitCode = 1;
			}
		}
	}
	catch (const GenericException &e)  																									// Provides Basler error handling.
	{
		cerr << "An exception occurred." << endl
			<< e.GetDescription() << endl;
		exitCode = 1;
	}
	PylonTerminate();
	return exitCode;
}

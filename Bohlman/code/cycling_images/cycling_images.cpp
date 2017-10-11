/*! \file cycle_exposures.cpp
\brief A documented file that takes and writes an image for 10 different exposures
Initializes the pylon resources, takes the photos, and passes a struct to write_basler_fits() function with image data in it
*/

#include "write_basler_fits.h"
#include <csignal>

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
	//signal(SIGINT, signalHandler);	
	int exitCode = 0;
	int i = 0;
	int exposure = 5000;
	PylonInitialize();  										// Initializes pylon runtime before using any pylon methods
	try
	{
		CDeviceInfo info;									// Set up attached Basler USB camera
		info.SetDeviceClass(Camera_t::DeviceClass());
		Camera_t camera(CTlFactory::GetInstance().CreateFirstDevice());   			// Creates an instant camera object with the camera device found first
		CGrabResultPtr ptrGrabResult;
		string file_name = (string)camera.GetDeviceInfo().GetModelName() + " " + (string)camera.GetDeviceInfo().GetSerialNumber();
		char* newstr = &file_name[0u];							// Gets camera information							// Gets the desired 

		camera.Open();									// Opens camera parameters
		camera.ExposureAuto.SetValue(ExposureAuto_Off);					// Sets up exposure time from above value
		camera.ExposureTime.SetValue(exposure);
		camera.Close();
		camera.StartGrabbing();  
		while (1) {
			if (i == 10) i = 0;			
										// Starts the grabbing of a singular image
			int tempcam = (int)camera.Basler_UsbCameraParams::CUsbCameraParams_Params::DeviceTemperature.GetValue();
			camera.RetrieveResult(5000, ptrGrabResult, TimeoutHandling_ThrowException);  	// Waits for an image and then retrieves it. A timeout of 5000 ms is used
			if (ptrGrabResult->GrabSucceeded())  						// If image is grabbed successfully 
			{
				struct image *cam_image = new struct image; 				// Setting up image struct
				cam_image->imgGrab = ptrGrabResult;
				cam_image->exposure = exposure;
				cam_image->temp = tempcam;
				cam_image->camname = newstr;
				char real_filename[25];							// Construct file name and store it in the struct
				strncpy(real_filename, "!", sizeof(real_filename));
				strcat(real_filename, "fitsimg");
				char exp_str[6];
				char num_str[6];
				sprintf(num_str, "_%d", i);
				strcat(real_filename, num_str);
				strcat(real_filename, ".fits");
				cam_image->imgname = real_filename;

				if (write_basler_fits(cam_image) != 0)  				//if image building from struct did not work
				{
					throw "Bad process in fits image writing!";
					delete(cam_image);
				}
				else {									//if image building from struct did work
					cout << "Image grab and write successful" << endl;
					delete(cam_image);
				}
				i++;
			}
			else  										// If image is not grabbed successfully, throw an error
			{
				cerr << "Error: " << ptrGrabResult->GetErrorCode() << " " << ptrGrabResult->GetErrorDescription() << endl;
				exitCode = 1;
			}
		}
	}
	catch (const GenericException &e)  								// Provides Basler error handling.
	{
		cerr << "An exception occurred." << endl
			<< e.GetDescription() << endl;
		exitCode = 1;
	}
	PylonTerminate();
	return exitCode;
}

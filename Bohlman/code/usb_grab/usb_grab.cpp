/*! \file usb_grab.cpp
\brief A documented file that reads from a camera and writes image
Reads image from a usb camera and writes info to a fits file for a given exposure time.
Exposure can be adjusted.
*/

#include "write_basler_fits.h"

int exitCode = 0;

//  Returns the value of the desired exposure of the camera form stdin.   
/** Returns value of exposure from cin (user input). Basler will sanity check this value for us.
*/
int get_exposure() 
{
	int exposure;  										// Holds exposure time from stdin
	cin >> exposure;
	std::cin.ignore(std::cin.rdbuf()->in_avail());
	return exposure;  									// Returns exposure time
}

//  Main function
/** param  argc An integer argument count of the command line arguments
* param  argv An argument vector of the command line arguments
* \return an integer: 0 upon exit success, 1 otherwise
*/
int main(int argc, ///< [in] the integer value of the count of the command line arguments
	char* argv[] ///< [ch] the integer value of the count of the command line arguments
)
{
	PylonInitialize();  									// Initializes pylon at runtime before using any pylon methods
	try
	{
		int exposure = get_exposure();  						// Gets the desired exposure time from function get_exposure() 

		CDeviceInfo info;								// Set up attached Basler USB camera
		info.SetDeviceClass(Camera_t::DeviceClass());
		CGrabResultPtr ptrGrabResult;
		Camera_t camera(CTlFactory::GetInstance().CreateFirstDevice()); 		// Creates an instant camera object with the camera device found first.
		camera.Open();  								// Opens camera parameters
		camera.ExposureAuto.SetValue(ExposureAuto_Off);					// Sets up exposure time from given value
		camera.ExposureTime.SetValue(exposure);

		string file_name = (string) camera.GetDeviceInfo().GetModelName() + " "+ (string) camera.GetDeviceInfo().GetSerialNumber();
		char* newstr = &file_name[0u];
		camera.StartGrabbing(1);  							// Starts the grabbing of a singular image
		int tempcam = (int)camera.Basler_UsbCameraParams::CUsbCameraParams_Params::DeviceTemperature.GetValue();
		camera.RetrieveResult(5000, ptrGrabResult, TimeoutHandling_ThrowException);  	// Waits for an image and then retrieves it. A timeout of 5000 ms is used
		camera.Close();
		if (ptrGrabResult->GrabSucceeded())  						// If image is grabbed successfully 
		{
			char real_filename[25];							// Construct file name from exposure time
			strncpy(real_filename, "!", sizeof(real_filename));
			strcat(real_filename, "fitsimg_exp");
			char exp_str[6];
			sprintf(exp_str, "%d", exposure);
			strcat(real_filename, exp_str);
			strcat(real_filename, ".fits");

			struct image *cam_image = new struct image; 				// Construct image struct and set up parameters
			cam_image->imgGrab = ptrGrabResult;
			cam_image->exposure = exposure;
			cam_image->temp = tempcam;
			cam_image->imgname = real_filename;
			cam_image->camname = newstr;

			if (write_basler_fits(cam_image) != 0)  				// If image building from struct did not work
			{
				throw "Bad process in fits image writing!";
			}
			else {									// If image building from struct did work
				cout << "Image grab and write successful" << endl;
				delete(cam_image);
			}
		}
		else  										// If image is not grabbed successfully, throw an error
		{
			cerr << "Error: " << ptrGrabResult->GetErrorCode() << " " << ptrGrabResult->GetErrorDescription() << endl;
			exitCode = 1;
		}
	}
	catch (const GenericException &e)  							// Provides Basler error handling.
	{
		cerr << "An exception occurred." << endl
			<< e.GetDescription() << endl;
		exitCode = 1;
	}
	PylonTerminate();   									// Releases all pylon resources. 
	return exitCode;
}

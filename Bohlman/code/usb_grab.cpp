/*! \file usb_grab.cpp
\brief A documented file that reads from a camera
Reads from a usb camera and delivers info.
Exposure can be adjusted.
*/

/*! \headerfile stdafx.h
\brief A header file that eases compilation with Visual Studio
A precompiled header required in VisualStudio that really helps speed up compilation time of the program.
*/

#include "stdafx.h"
#include "write_basler_fits.h"

int exitCode = 0;

/*! \struct image
//struct that is passed to write_basler_fits() that has data for the image
* holds the int value for exposure, int value for temperature of camera, pointer to image, and camera model name/serial number
*/

//  Returns the value of the desired exposure of the camera form stdin.   
/** More information about the function.  Note since its void I don't have a return flag.
*  However, this function shouldn't be void.  It ought to communicate status of the fits calls, etc.
*/
int get_exposure() 
{
	int exposure;  // Holds exposure time from stdin
	cin >> exposure;
	std::cin.ignore(std::cin.rdbuf()->in_avail());
	return exposure;  // Returns exposure time
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
	std::ios_base::sync_with_stdio(false);  	// Seperates std from stdio
	PylonInitialize();  	// Initializes pylon runtime before using any pylon methods
	try
	{
		int exposure = get_exposure();  // Gets the desired exposure time from function get_exposure() 

		CDeviceInfo info;
		info.SetDeviceClass(Camera_t::DeviceClass());
		CGrabResultPtr ptrGrabResult;
		Camera_t camera(CTlFactory::GetInstance().CreateFirstDevice());   // Creates an instant camera object with the camera device found first.
		camera.Open();  		// Starts the grabbing of c_countOfImagesToGrab images.
		camera.ExposureAuto.SetValue(ExposureAuto_Off);
		camera.ExposureTime.SetValue(exposure);

		string file_name = (string) camera.GetDeviceInfo().GetModelName() + " "+ (string) camera.GetDeviceInfo().GetSerialNumber();
		char* newstr = &file_name[0u];
		camera.StartGrabbing(1);  		// Starts the grabbing of c_countOfImagesToGrab images.
		int tempcam = (int)camera.Basler_UsbCameraParams::CUsbCameraParams_Params::DeviceTemperature.GetValue();
		camera.RetrieveResult(5000, ptrGrabResult, TimeoutHandling_ThrowException);  // Waits for an image and then retrieves it. A timeout of 5000 ms is used.
		camera.Close();
		if (ptrGrabResult->GrabSucceeded())  // If image is grabbed successfully: 
		{
			char real_filename[25];
			strncpy(real_filename, "!", sizeof(real_filename));
			strcat(real_filename, "fitsimg_exp");
			char exp_str[6];
			sprintf(exp_str, "%d", exposure);
			strcat(real_filename, exp_str);
			strcat(real_filename, ".fits");

			struct image *cam_image = new struct image; //Setting up image struct
			cam_image->imgGrab = ptrGrabResult;
			cam_image->exposure = exposure;
			cam_image->temp = tempcam;
			cam_image->imgname = real_filename;
			cam_image->camname = newstr;

			if (write_basler_fits(cam_image) != 0)  //if image building did not work
			{
				throw "Bad process in fits image writing!";
			}
			else {									//if image building did work
				cout << "Image grab and write successful" << endl;
				delete(cam_image);
			}
		}
		else  // If image is not grabbed successfully
		{
			cout << "Error: " << ptrGrabResult->GetErrorCode() << " " << ptrGrabResult->GetErrorDescription() << endl; //Prints an error
		}
	}
	catch (const GenericException &e)  // Provides error handling.
	{
		cerr << "An exception occurred." << endl
			<< e.GetDescription() << endl;
		exitCode = 1;
	}
	cerr << endl << "Press Enter to exit." << endl;
	while (cin.get() != '\n');
	PylonTerminate();   // Releases all pylon resources. 
	return exitCode;
}
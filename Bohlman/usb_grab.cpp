/*! \file usb_grab.cpp
\brief A documented file that reads from a camera
Reads from a usb camera and delivers info.
Exposure can be adjusted.
*/

/*! \headerfile stdafx.h
\brief A header file that eases compilation with Visual Studio
A precompiled header required in VisualStudio that really helps speed up compilation time of the program.
*/

/*! \headerfile pylon/PylonIncludes.h
\brief A header file that reads in Pylon API
Details.
*/

/*! \headerfile pylon/PylonGUI.h
\brief A header file
Details.
*/

/*! \headerfile pylon/usb/BaslerUsbInstantCamera.h
\brief A header file
Details.
*/

/*! \headerfile pylon/usb/_BaslerUsbCameraParams.h
\brief A header file
Details.
*/

/*! \headerfile GenApi/IFloat.h
\brief A header file
Details.
*/

/*! \var c_countOfImagesToGrab
\brief Number of images to grab
Details.
*/

/*! \var exitCode
\brief what code to exit with, initialized at 0
Details.
*/

#include "stdafx.h"
#include <pylon/PylonIncludes.h>
#include <pylon/usb/BaslerUsbInstantCamera.h>
#include <pylon/usb/_BaslerUsbCameraParams.h>
#include <GenApi/IFloat.h>
#include "fitsio.h"
#include <pylon/PixelData.h>
#include <iostream>
#include <pylon/GrabResultData.h>
#ifdef PYLON_WIN_BUILD
#    include <pylon/PylonGUI.h>
#endif

using namespace Pylon;
using namespace std;
static const uint32_t c_countOfImagesToGrab = 1;
int exitCode = 0;

/*! \struct image
//struct that is passed to write_basler_fits() that has data for the image
* holds the int value for exposure, int value for temperature of camera, pointer to image, and camera model name/serial number
*/
struct image {
	int exposure;
	int temp;
	CGrabResultPtr imgGrab;
	char* camname;
};

//  Writes fits file of the image stored in memory   
/** More information about the function.  Note since its void I don't have a return flag.
*  \return an integer: 0 upon exit success, 1 otherwise
*/
int write_basler_fits(struct image *cam_image)
{
#ifdef PYLON_WIN_BUILD
	uint8_t *pImageBuffer = (uint8_t *)cam_image->imgGrab->GetBuffer();  // Gets image buffer from pointer to image data
#endif		
	fitsfile *fptr;       //Creates pointer to the FITS file; defined in fitsio.h   
	int width = (int)cam_image->imgGrab->GetWidth();
	int height = (int)cam_image->imgGrab->GetHeight();
	long  fpixel = 1, naxis = 2;
	long naxes[2] = { width, height };

	if (fits_create_file(&fptr, "!testfile.fits", &exitCode) != 0) //Creates new fits file
	{
		fits_report_error(stderr, exitCode);  // Prints out any fits error messages
		return 1;
	}
	if (fits_create_img(fptr, BYTE_IMG, naxis, naxes, &exitCode) != 0)  //Creates the primary array image
	{
		fits_report_error(stderr, exitCode);  // Prints out any fits error messages
		return 1;
	}
	if (fits_update_key(fptr, TLONG, "EXPOSURE", &(cam_image->exposure), "Total Exposure Time", &exitCode) != 0)  //Writes exprosure keyword of image from struct
	{
		fits_report_error(stderr, exitCode);  // Prints out any fits error messages
		return 1;
	}
	if (fits_update_key(fptr, TSTRING, "INSTRUME", cam_image->camname, "Model of camera followed by serial number", &exitCode) != 0)  //Writes camera name followed by serial number keyword from struct
	{
		fits_report_error(stderr, exitCode);  // Prints out any fits error messages
		return 1;
	}
	if (fits_update_key(fptr, TINT, "TEMP",&(cam_image->temp), "Temperature of camera in celsius when picture was taken", &exitCode) != 0)  //Writes camera temperature keyword from struct
	{
		fits_report_error(stderr, exitCode);  // Prints out any fits error messages
		return 1;
	}
	if (fits_write_date(fptr, &exitCode) != 0)
	{
		fits_report_error(stderr, exitCode);  // Prints out any fits error messages
		return 1;
	}
	/*
	int ii, jj;
	for (jj = 0; jj < naxes[1]; ++jj)										///Applies image buffer pixel values to an array representing the image
		for (ii = 0; ii < naxes[0]; ++ii)									
			array[jj*width + ii] = (uint32_t)pImageBuffer[jj*width + ii];
	fits_write_img(fptr, TBYTE, fpixel, width*height, array, &exitCode);    /// Writes the array of pixel values to the image
	 */
	if (fits_write_img(fptr, TBYTE, fpixel, width*height, pImageBuffer, &exitCode) != 0)  // Writes pointer values to the image
	{
		fits_report_error(stderr, exitCode);  // Prints out any fits error messages
		return 1;
	}
	if (fits_close_file(fptr, &exitCode) != 0) // Closes the fits file
	{
		fits_report_error(stderr, exitCode);  // Prints out any fits error messages
		return 1;
	}
	return 0;
}

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
		CBaslerUsbInstantCamera camera(CTlFactory::GetInstance().CreateFirstDevice());   // Creates an instant camera object with the camera device found first.
		string file_name = (string) camera.GetDeviceInfo().GetModelName() + " "+ (string) camera.GetDeviceInfo().GetSerialNumber();
		char* newstr = &file_name[0u];
		camera.StartGrabbing(c_countOfImagesToGrab);  		// Starts the grabbing of c_countOfImagesToGrab images.
		int exposure = get_exposure();  // Gets the desired exposure time from function get_exposure() 
		CGrabResultPtr ptrGrabResult;

		while (camera.IsGrabbing())
		{
			camera.Basler_UsbCameraParams::CUsbCameraParams_Params::ExposureTime.SetValue(exposure);   // Sets the exposure of the next camera shot.
			int tempcam = (int)camera.Basler_UsbCameraParams::CUsbCameraParams_Params::DeviceTemperature.GetValue();
			camera.RetrieveResult(5000, ptrGrabResult, TimeoutHandling_ThrowException);  // Waits for an image and then retrieves it. A timeout of 5000 ms is used.
			if (ptrGrabResult->GrabSucceeded())  // If image is grabbed successfully: 
			{
				struct image *cam_image = new struct image; //Setting up image struct
				cam_image->imgGrab = ptrGrabResult;
				cam_image->exposure = exposure;
				cam_image->temp = tempcam;
				cam_image->camname = _strdup(newstr);
				if (write_basler_fits(cam_image) != 0)  //if image building did not work
				{
					throw "Bad process in fits image writing!";
				}
				else {									//if image building did work
					cout << "Image grab and write successful" << endl;
					free(cam_image->camname);
					delete(cam_image);
				}
			}
			else  // If image is not grabbed successfully
			{
				cout << "Error: " << ptrGrabResult->GetErrorCode() << " " << ptrGrabResult->GetErrorDescription() << endl; //Prints an error
			}
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
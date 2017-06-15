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

/*! \namespace Pylon
\brief Namespace for using pylon objects
Details.
*/

/*! \namespace std
\brief Namespace for using cout.
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

/*! \fn get_exposure
\brief  Gets the exposure from stdin and clears buffer
\return the value of the exposure inputted
*/

/*! \fn build_image
\brief  writes tiff file and fits file of the image stored in memory
\param  ptrGrabResult points to image in memory
\param  exposure the integer value of the exposure of the image
\return void
*/

/*! \fn write_fits
\brief  writes fits file of the image stored in memory
\param  exposure the integer value of the exposure of the image
\param  width width in pixels of image
\param  height height in pixels of image
\param  pImageBuffer points to buffer in memory of the image
\return void
*/

/*! \fn main
\brief  Main function
\param  argc An integer argument count of the command line arguments
\param  argv An argument vector of the command line arguments
\return an integer 0 upon exit success
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

void write_fits(int exposure, int width, int height, uint8_t *pImageBuffer) {
	fitsfile *fptr;       ///Creates pointer to the FITS file; defined in fitsio.h
	long  fpixel = 1, naxis = 2;
	long naxes[2] = { width, height };

	fits_create_file(&fptr, "!testfile.fits", &exitCode);   ///Creates new fits file
	fits_create_img(fptr, BYTE_IMG, naxis, naxes, &exitCode);  ///Creates the primary array image
	fits_update_key(fptr, TLONG, "EXPOSURE", &exposure, "Total Exposure Time", &exitCode); ///Writes exprosure keyword; must pass the ADDRESS of the value
	/*
	int ii, jj;
	for (jj = 0; jj < naxes[1]; jj++)										///Applies image buffer pixel values to an array representing 
		for (ii = 0; ii < naxes[0]; ii++)									the image
			array[jj*width + ii] = (uint32_t)pImageBuffer[jj*width + ii];  
	fits_write_img(fptr, TBYTE, fpixel, width*height, array, &exitCode);   /// Writes the array of pixel values to the image
	*/
	fits_write_img(fptr, TBYTE, fpixel, width*height, pImageBuffer, &exitCode); /// Writes pointer values to the image
	fits_close_file(fptr, &exitCode);            /// Closes the fits file
	fits_report_error(stderr, exitCode);  /// Prints out any fits error messages
}

int get_exposure() {
	int exposure;  /// Holds exposure time from stdin
	cin >> exposure;
	std::cin.ignore(std::cin.rdbuf()->in_avail());
	return exposure;  /// Returns exposure time
}

void build_image(CGrabResultPtr ptrGrabResult, int exposure) {
#ifdef PYLON_WIN_BUILD
	uint8_t *pImageBuffer = (uint8_t *)ptrGrabResult->GetBuffer();  /// Gets image buffer from pointer to image data
	Pylon::DisplayImage(1, ptrGrabResult);  /// Displays the grabbed image.
	CImagePersistence::Save(ImageFileFormat_Tiff, "GrabbedImage.tiff", ptrGrabResult);  /// Saves image as a tiff file
	CPylonImage image;
	image.AttachGrabResultBuffer(ptrGrabResult);  /// Initializes a CPylonImage object with the buffer from the grab result.
#endif			
	int width = (int)ptrGrabResult->GetWidth();
	int height = (int)ptrGrabResult->GetHeight();
	write_fits(exposure, width, height, pImageBuffer); ///Writes a fits file with function write_fits()
}

int main(int argc, char* argv[])
{
	std::ios_base::sync_with_stdio(false);  	/// Seperates std from stdio
	PylonInitialize();  	/// Initializes pylon runtime before using any pylon methods
	try
	{
		CBaslerUsbInstantCamera camera(CTlFactory::GetInstance().CreateFirstDevice());   /// Creates an instant camera object with the camera device found first.
		cout << "Using device " << camera.GetDeviceInfo().GetModelName() << endl;  /// Prints the model name of the camera.
		int exposure = get_exposure();  /// Gets the desired exposure time from function get_exposure() 
		camera.StartGrabbing(c_countOfImagesToGrab);  		/// Starts the grabbing of c_countOfImagesToGrab images.
		CGrabResultPtr ptrGrabResult;
		while (camera.IsGrabbing())
		{
			camera.Basler_UsbCameraParams::CUsbCameraParams_Params::ExposureTime.SetValue(exposure);   ///Sets the exposure of the next camera shot.
			camera.RetrieveResult(5000, ptrGrabResult, TimeoutHandling_ThrowException);  /// Waits for an image and then retrieves it. A timeout of 5000 ms is used.
			if (ptrGrabResult->GrabSucceeded())  /// If image is grabbed successfully: 
			{
				build_image(ptrGrabResult, exposure);  ///Image files are built with build_image()
			}
			else  /// If image is not grabbed successfully
			{
				cout << "Error: " << ptrGrabResult->GetErrorCode() << " " << ptrGrabResult->GetErrorDescription() << endl; ///Prints an error
			}
		}
	}
	catch (const GenericException &e)  /// Provides error handling.
	{
		cerr << "An exception occurred." << endl
			<< e.GetDescription() << endl;
		exitCode = 1;
	}
	cerr << endl << "Press Enter to exit." << endl;
	while (cin.get() != '\n');
	PylonTerminate();   /// Releases all pylon resources. 
	return exitCode;
}
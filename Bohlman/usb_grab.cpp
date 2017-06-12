/*! \file usb_grab.cpp
\brief A documented file that reads from a camera

Reads from a usb camera and delivers info.
Exposure can be adjusted.
*/

/*! \headerfile stdafx.h
\brief A header file that eases compilation with Visual Studio

Details.
*/

/*! \headerfile pylon/PylonIncludes.h
\brief A header file that reads in Pylon API

Details.
*/

/*! \def PYLON_WIN_BUILD
\brief defines something for pylon/PylonGUI.h

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
\brief Namespace for using cout.

Details.
*/

/*! \fn main
\brief  Main function
\param  argc An integer argument count of the command line arguments
\param  argv An argument vector of the command line arguments
\return an integer 0 upon exit success
*/

#include "stdafx.h"
#include <pylon/PylonIncludes.h>
#ifdef PYLON_WIN_BUILD
#    include <pylon/PylonGUI.h>
#endif
#include <pylon/usb/BaslerUsbInstantCamera.h>
#include <pylon/usb/_BaslerUsbCameraParams.h>
#include <GenApi/IFloat.h>
#include "fitsio.h"
#include <pylon/PixelData.h>
#include <iostream>
#include <pylon/GrabResultData.h>
using namespace Pylon;
using namespace std;
static const uint32_t c_countOfImagesToGrab = 1;

int main(int argc, char* argv[])
{
	int exitCode = 0;
	std::ios_base::sync_with_stdio(false);  	//seperate std from io
	PylonInitialize();  	// Before using any pylon methods, the pylon runtime must be initialized. 
	try
	{
		CBaslerUsbInstantCamera camera(CTlFactory::GetInstance().CreateFirstDevice());   // Create an instant camera object with the camera device found first.
		cout << "Using device " << camera.GetDeviceInfo().GetModelName() << endl;  // Print the model name of the camera.
		int exposure;
		cin >> exposure;
		std::cin.ignore(std::cin.rdbuf()->in_avail());
		camera.StartGrabbing(c_countOfImagesToGrab);  		// Start the grabbing of c_countOfImagesToGrab images.
		CGrabResultPtr ptrGrabResult;  // This smart pointer will receive the grab result data.

		while (camera.IsGrabbing())
		{
			camera.Basler_UsbCameraParams::CUsbCameraParams_Params::ExposureTime.SetValue(exposure);   //Sets the exposure of the next camera shot.
			camera.RetrieveResult(5000, ptrGrabResult, TimeoutHandling_ThrowException);  // Wait for an image and then retrieve it. A timeout of 5000 ms is used.
			if (ptrGrabResult->GrabSucceeded())  // Image grabbed successfully?
			{
#ifdef PYLON_WIN_BUILD
				uint8_t *pImageBuffer = (uint8_t *)ptrGrabResult->GetBuffer();
				cout << "Gray value of first pixel: " << (uint32_t)pImageBuffer[0] << endl << endl;
				//Pylon::DisplayImage(1, ptrGrabResult);  // Display the grabbed image.
				CImagePersistence::Save(ImageFileFormat_Tiff, "GrabbedImage.tiff", ptrGrabResult);
				CPylonImage image;
				image.AttachGrabResultBuffer(ptrGrabResult);  // Initializes the image object with the buffer from the grab result.
#endif			
				//To start with writing the fits file
				fitsfile *fptr;       /* pointer to the FITS file; defined in fitsio.h */
				int width = (int) ptrGrabResult->GetWidth();
				int height = (int) ptrGrabResult->GetHeight();
				int ii, jj;
				long  fpixel = 1, naxis = 2;
				long naxes[2] = { width, height }; 
				int* array = new int[width*height];

				fits_create_file(&fptr,"!testfile.fits", &exitCode);   //Create new file
				fits_create_img(fptr, LONG_IMG, naxis, naxes, &exitCode);  //Create the primary array image
				fits_update_key(fptr, TLONG, "EXPOSURE", &exposure, "Total Exposure Time", &exitCode); //Write a keyword; must pass the ADDRESS of the value
				/*
				for (jj = 0; jj < naxes[1]; jj++)
					for (ii = 0; ii < naxes[0]; ii++)
						array[jj*width + ii] = (int)image.GetPixelData(ii, jj).Data.Mono;
				*/
				
				
				for (jj = 0; jj < naxes[1]; jj++)
					for (ii = 0; ii < naxes[0]; ii++)
						array[jj*width + ii] = (uint32_t)pImageBuffer[jj*width + ii];
				
				fits_write_img(fptr, TLONG, fpixel, width*height, array, &exitCode); // Write the array of integers to the image
				fits_close_file(fptr, &exitCode);            /* close the file */
				fits_report_error(stderr, exitCode);  /* print out any error messages */
			}
			else
			{
				cout << "Error: " << ptrGrabResult->GetErrorCode() << " " << ptrGrabResult->GetErrorDescription() << endl;
			}
		}
	}
	catch (const GenericException &e)  // Error handling.
	{	
		cerr << "An exception occurred." << endl
			<< e.GetDescription() << endl;
		exitCode = 1;
	}
	cerr << endl << "Press Enter to exit." << endl;  //Press enter to exit
	while (cin.get() != '\n');
	PylonTerminate();   // Releases all pylon resources. 
	return exitCode;
}
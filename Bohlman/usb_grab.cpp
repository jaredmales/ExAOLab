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

using namespace Pylon;
using namespace std;
static const uint32_t c_countOfImagesToGrab = 1;

int main(int argc, char* argv[])
{
	int exitCode = 0;

	// Before using any pylon methods, the pylon runtime must be initialized. 
	PylonInitialize();
	try
	{
		// Create an instant camera object with the camera device found first.
		CBaslerUsbInstantCamera camera(CTlFactory::GetInstance().CreateFirstDevice());

		// Print the model name of the camera.
		cout << "Using device " << camera.GetDeviceInfo().GetModelName() << endl;
		int exposure;
		cin >> exposure;

		// Start the grabbing of c_countOfImagesToGrab images.
		// The camera device is parameterized with a default configuration which
		// sets up free-running continuous acquisition.
		camera.StartGrabbing(c_countOfImagesToGrab);

		// This smart pointer will receive the grab result data.
		CGrabResultPtr ptrGrabResult;

		// Camera.StopGrabbing() is called automatically by the RetrieveResult() method
		// when c_countOfImagesToGrab images have been retrieved.
		while (camera.IsGrabbing())
		{
			//Sets the exposure of the next camera shot. Default is 5000.
			camera.Basler_UsbCameraParams::CUsbCameraParams_Params::ExposureTime.SetValue(exposure);
			// Wait for an image and then retrieve it. A timeout of 5000 ms is used.
			camera.RetrieveResult(5000, ptrGrabResult, TimeoutHandling_ThrowException);
			// Image grabbed successfully?
			if (ptrGrabResult->GrabSucceeded())
			{
#ifdef PYLON_WIN_BUILD
				// Display the grabbed image.
				Pylon::DisplayImage(1, ptrGrabResult);
				CImagePersistence::Save(ImageFileFormat_Tiff, "GrabbedImage.tiff", ptrGrabResult);
				CPylonImage image;
				// Initializes the image object with the buffer from the grab result.
				// This prevents the reuse of the buffer for grabbing as long as it is
				// not released.
				// Please note that this is not relevant for this example because the
				// camera object has been destroyed already.
				image.AttachGrabResultBuffer(ptrGrabResult);
#endif
				
				//To start with writing the fits file
				fitsfile *fptr;       /* pointer to the FITS file; defined in fitsio.h */
				int width = (int) ptrGrabResult->GetWidth();
				int height = (int) ptrGrabResult->GetHeight();
				int status, ii, jj;
				long  fpixel = 1, naxis = 2, nelements;
				long naxes[2] = { width, height }; 

				cout << width << endl;
				cout << height << endl;

				int* array = new int[width*height];

				int fits1 = fits_create_file(&fptr,"!testfile.fits", &exitCode);   /* create new file */
				cout << fits1 << endl;
				Sleep(5000);

				/* Create the primary array image (16-bit short integer pixels */
				int fits2 = fits_create_img(fptr, LONG_IMG, naxis, naxes, &status);
				cout << fits2 << endl;
				Sleep(5000);

				/* Write a keyword; must pass the ADDRESS of the value */
				fits_update_key(fptr, TLONG, "EXPOSURE", &exposure,
					"Total Exposure Time", &status);

				/* Initialize the values in the image with a linear ramp function */
				for (jj = 0; jj < naxes[1]; jj++)
					for (ii = 0; ii < naxes[0]; ii++)
						array[jj*width + ii] = (int)image.GetPixelData(jj, ii).PixelDataType_YUV;
						//cout << "hey!" << endl;

				nelements = naxes[0] * naxes[1];          /* number of pixels to write */
				Sleep(5000);
														  /* Write the array of integers to the image */
				int fits3 = fits_write_img(fptr, TSHORT, fpixel, nelements, array, &status);
				Sleep(5000);
				cout << fits3 << endl;
				Sleep(5000);

				fits_close_file(fptr, &status);            /* close the file */

				fits_report_error(stderr, status);  /* print out any error messages */
			}
			else
			{
				cout << "Error: " << ptrGrabResult->GetErrorCode() << " " << ptrGrabResult->GetErrorDescription() << endl;
				Sleep(5000);
			}
		}
	}
	catch (const GenericException &e)
	{
		// Error handling.
		cerr << "An exception occurred." << endl
			<< e.GetDescription() << endl;
		Sleep(5000);
		exitCode = 1;
	}
	
	// Comment the following two lines to disable waiting on exit.
	cerr << endl << "Press Enter to exit." << endl;
	while (cin.get() != '\n');

	// Releases all pylon resources. 
	PylonTerminate();
}
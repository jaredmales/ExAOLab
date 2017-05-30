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
			camera.Basler_UsbCameraParams::CUsbCameraParams_Params::ExposureTime.SetValue(2000);
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
			}
			else
			{
				cout << "Error: " << ptrGrabResult->GetErrorCode() << " " << ptrGrabResult->GetErrorDescription() << endl;
			}
		}
	}
	catch (const GenericException &e)
	{
		// Error handling.
		cerr << "An exception occurred." << endl
			<< e.GetDescription() << endl;
		exitCode = 1;
	}
	
	// Comment the following two lines to disable waiting on exit.
	cerr << endl << "Press Enter to exit." << endl;
	while (cin.get() != '\n');

	// Releases all pylon resources. 
	PylonTerminate();
}
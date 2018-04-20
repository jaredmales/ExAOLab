/*! \file cycle_exposures.cpp
\brief A documented file that takes command line arguments for exposure and number of images to grab and outputs fits files
Initializes the pylon resources, takes the photos, and passes a struct to write_basler_fits() function with image data in it
*/

#include "write_basler_fits.h"
//  Subtracts a median image from the given file name
/** Gets the name of the file, opens it, opens the median file, subtracts the median file from the given file name, writes the new file.
*  \return an integer: 0 upon exit success, 1 otherwise
*/

//  Main function
/** Initializes pylon resources, takes pictures, closes all pylon resources.
* \return an integer: 0 upon exit success, 1 otherwise
*/
int main(int argc, ///< [in] the integer value of the count of the command line arguments
	char* argv[] ///< [ch.ar] the integer value of the count of the command line arguments
)
{
	int c_countOfImagesToGrab, z = 0;
	uint32_t countOfImagesToGrab;
	int exitCode = 0;
	int exposure = 0;
	if (argc == 3) {																															// if there are more than 2 command line arguments
		exposure = atoi(argv[1]);																														// exposure is first argument																														
		countOfImagesToGrab = atoi(argv[2]);																									// number of images to grab is second argument
	}
	else {																																	// if there are less than 2 command line arguments
		exposure = 1000;																															// default value of exp: 5000 us
		countOfImagesToGrab = 10;																											// default number of images to grab: 10
	}			
	PylonInitialize();  																													// Initializes pylon runtime before using any pylon methods
	try
	{
		CDeviceInfo info;																													// Set up attached Basler USB camera
		info.SetDeviceClass(Camera_t::DeviceClass());
		CBaslerUsbInstantCamera camera( CTlFactory::GetInstance().CreateFirstDevice());
		string file_name = (string)camera.GetDeviceInfo().GetModelName() + " " + (string)camera.GetDeviceInfo().GetSerialNumber();		// Gets camera model name and serial number
		char* newstr = &file_name[0u];																									// Casts string as char* for cfitsio
		camera.Open();																														// Opens camera parameters to grab images and set exposure time
		camera.ExposureAuto.SetValue(ExposureAuto_Off);																						// Set exposure
		camera.ExposureTime.SetValue(exposure);
		int camera_iteration = 1;
		int nelements = 640 * 480;
		int j, k;																						// Subtract median image from passed file and store value into new array
		int width = 640, height = 480;

		camera.StartGrabbing(countOfImagesToGrab);																							// Start grabbing a provided amount of images
		while (camera.IsGrabbing()) {

			CGrabResultPtr ptrGrabResult;
			int tempcam = (int)camera.DeviceTemperature.GetValue();																			// Gets and stores temperature of camera
			camera.RetrieveResult(5000, ptrGrabResult, TimeoutHandling_ThrowException);  													// Waits for an image and then retrieves it. A timeout of 5000 ms is used
			if (ptrGrabResult->GrabSucceeded())  																							// If image is grabbed successfully 
			{
				uint8_t *pImageBuffer = (uint8_t *)ptrGrabResult->GetBuffer();

				vector <int> average;
				
				for (k = 235; k < 245; k = k + 1) 
				{
					for (j = 315; j < 325; j = j + 1) 
					{
						average.push_back((int) pImageBuffer[k*width + j]);
						//cout << (int) pImageBuffer[k*width + j] << endl;
					}
				}

				cout << camera_iteration << '\t' << accumulate(average.begin(), average.end(), 0.0)/average.size() << endl;
			}
			else  																															// If image is not grabbed successfully, throw an error
			{
				cerr << "Error: " << ptrGrabResult->GetErrorCode() << " " << ptrGrabResult->GetErrorDescription() << endl;
				exitCode = 1;
			}
			
			camera_iteration++;
		}
		camera.Close();	
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

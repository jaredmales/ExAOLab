/*! \file cycle_exposures.cpp
\brief A documented file that takes and writes an image for 10 different exposures
Initializes the pylon resources, takes the photos, and passes a struct to write_basler_fits() function with image data in it
*/

#include "stdafx.h"
#include "write_basler_fits.h"

//  Main function
/** Initializes pylon resources, takes pictures, closes all pylon resources.
* \return an integer: 0 upon exit success, 1 otherwise
*/
int main(int argc, ///< [in] the integer value of the count of the command line arguments
	char* argv[] ///< [ch.ar] the integer value of the count of the command line arguments
)
{
	int exitCode = 0;
	int expArray[10] = { 1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 9000, 10000 };
	int i = 0;
	std::ios_base::sync_with_stdio(false);  	// Seperates std from stdio
	PylonInitialize();  	// Initializes pylon runtime before using any pylon methods
	try
	{
		for (int j = 0; j < 10; ++j)
		{
			CDeviceInfo info;
			info.SetDeviceClass(Camera_t::DeviceClass());
			Camera_t camera(CTlFactory::GetInstance().CreateFirstDevice());   // Creates an instant camera object with the camera device found first.
			string file_name = (string)camera.GetDeviceInfo().GetModelName() + " " + (string)camera.GetDeviceInfo().GetSerialNumber();
			char* newstr = &file_name[0u];
			CGrabResultPtr ptrGrabResult;
			camera.Open();
			camera.ExposureAuto.SetValue(ExposureAuto_Off);
			int exposure = expArray[j];  // Gets the desired exposure time from function get_exposure() 
			camera.ExposureTime.SetValue(exposure);
			
				camera.StartGrabbing(1);  		// Starts the grabbing of c_countOfImagesToGrab images.
				int tempcam = (int)camera.Basler_UsbCameraParams::CUsbCameraParams_Params::DeviceTemperature.GetValue();
				camera.RetrieveResult(5000, ptrGrabResult, TimeoutHandling_ThrowException);  // Waits for an image and then retrieves it. A timeout of 5000 ms is used.
				if (ptrGrabResult->GrabSucceeded())  // If image is grabbed successfully: 
				{
					struct image *cam_image = new struct image; //Setting up image struct
					cam_image->imgGrab = ptrGrabResult;
					cam_image->exposure = exposure;
					cam_image->temp = tempcam;
					cam_image->camname = _strdup(newstr);

					char real_filename[25];
					strncpy(real_filename, "!", sizeof(real_filename));
					strcat(real_filename, "fitsimg_exp");
					char exp_str[6];
					sprintf(exp_str, "%d", cam_image->exposure);
					strcat(real_filename, exp_str);
					char num_str[6];
					sprintf(num_str, "_%d", i);
					strcat(real_filename, num_str);
					strcat(real_filename, ".fits");
					cam_image->imgname = _strdup(real_filename);


					if (write_basler_fits(cam_image) != 0)  //if image building did not work
					{
						throw "Bad process in fits image writing!";
						free(cam_image->camname);
						delete(cam_image);
					}
					else {									//if image building did work
						cout << "Image grab and write successful" << endl;
						free(cam_image->camname);
						delete(cam_image);
					}
				}
				else  // If image is not grabbed successfully
				{
					cerr << "Error: " << ptrGrabResult->GetErrorCode() << " " << ptrGrabResult->GetErrorDescription() << endl; //Prints an error
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
	Pylon::PylonTerminate(true);   // Releases all pylon resources. 
	return exitCode;
}

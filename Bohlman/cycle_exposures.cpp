#include "stdafx.h"
#include "write_basler_fits.h"

//  Main function
/** param  argc An integer argument count of the command line arguments
* param  argv An argument vector of the command line arguments
* \return an integer: 0 upon exit success, 1 otherwise
*/
int main(int argc, ///< [in] the integer value of the count of the command line arguments
	char* argv[] ///< [ch] the integer value of the count of the command line arguments
)
{
	int exitCode = 0;
	int expArray[10] = { 1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 9000, 10000 };
	int i = 0;
	std::ios_base::sync_with_stdio(false);  	// Seperates std from stdio
	PylonInitialize();  	// Initializes pylon runtime before using any pylon methods
	try
	{
			CBaslerUsbInstantCamera camera(CTlFactory::GetInstance().CreateFirstDevice());   // Creates an instant camera object with the camera device found first.
			string file_name = (string)camera.GetDeviceInfo().GetModelName() + " " + (string)camera.GetDeviceInfo().GetSerialNumber();
			char* newstr = &file_name[0u];
			camera.StartGrabbing(10);  		// Starts the grabbing of c_countOfImagesToGrab images.
			CGrabResultPtr ptrGrabResult;
			while (camera.IsGrabbing())
			{
				int exposure = expArray[i];  // Gets the desired exposure time from function get_exposure() 
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
					cout << "Error: " << ptrGrabResult->GetErrorCode() << " " << ptrGrabResult->GetErrorDescription() << endl; //Prints an error
				}
				++i;
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

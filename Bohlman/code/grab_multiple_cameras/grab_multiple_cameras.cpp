
/*! \file grab_multiple_cameras.cpp
\brief A file that repeatedly grabs images from 2 cameras connected to system
*/

// Include files to use the PYLON API.
#include "write_basler_fits.h"
#include <pylon/usb/BaslerUsbInstantCameraArray.h>

static const size_t c_maxCamerasToUse = 2;
int main(int argc, char* argv[])
{
    // The exit code of the sample application.
    int exitCode = 0;
    // Before using any pylon methods, the pylon runtime must be initialized. 
    PylonInitialize();
    try
    {
        // Get the transport layer factory.
        CTlFactory& tlFactory = CTlFactory::GetInstance();
        // Get all attached devices and exit application if no device is found.
        DeviceInfoList_t devices;
        if ( tlFactory.EnumerateDevices(devices) == 0 )
        {
            throw RUNTIME_EXCEPTION( "No camera present.");
        }
        // Create an array of instant cameras for the found devices and avoid exceeding a maximum number of devices.
        //CInstantCameraArray cameras( min( devices.size(), c_maxCamerasToUse));
        CBaslerUsbInstantCameraArray cameras( min( devices.size(), c_maxCamerasToUse));

        // Create and attach all Pylon Devices.
        for ( size_t i = 0; i < cameras.GetSize(); ++i)
        {
            cameras[ i ].Attach( tlFactory.CreateDevice( devices[ i ]));
            // Print the model name of the camera.
            cout << "Using device " << cameras[ i ].GetDeviceInfo().GetModelName() << endl;
        }
        // Starts grabbing for all cameras starting with index 0. The grabbing
        // is started for one camera after the other. That's why the images of all
        // cameras are not taken at the same time.
        // However, a hardware trigger setup can be used to cause all cameras to grab images synchronously.
        // According to their default configuration, the cameras are
        // set up for free-running continuous acquisition.
        int expos = 4999;											//SET EXPOSURE HERE
        cameras.Open();
		cameras[0].ExposureAuto.SetValue(ExposureAuto_Off);
		cameras[0].ExposureTime.SetValue(expos);
		cameras[1].ExposureAuto.SetValue(ExposureAuto_Off);
		cameras[1].ExposureTime.SetValue(expos);
        cameras.StartGrabbing();
        // This smart pointer will receive the grab result data.
        CGrabResultPtr ptrGrabResult1;
		CGrabResultPtr ptrGrabResult2;
        // Grab c_countOfImagesToGrab from the cameras.
		int i = 0;
        while(cameras.IsGrabbing())
        {
            
	    cameras[0].RetrieveResult( 5000, ptrGrabResult1, TimeoutHandling_ThrowException);
	    cameras[1].RetrieveResult( 5000, ptrGrabResult2, TimeoutHandling_ThrowException);
	    // When the cameras in the array are created the camera context value
            // is set to the index of the camera in the array.
            // The camera context is a user settable value.
            // This value is attached to each grab result and can be used
            // to determine the camera that produced the grab result.
            //intptr_t cameraContextValue = ptrGrabResult1->GetCameraContext();
        if (ptrGrabResult1->GrabSucceeded())  																									// If image from camera 1 is grabbed successfully 
		{
			string file_name = (string) cameras[0].GetDeviceInfo().GetModelName() + " "+ (string) cameras[0].GetDeviceInfo().GetSerialNumber();	// Get device name from model name and serial number for first camera
			char* newstr = &file_name[0u];																										// Convert device name to char* for cfitsio
			int tempcam = (int)cameras[0].DeviceTemperature.GetValue();																			// Store device temperature
			int exposure = (int)cameras[0].ExposureTime.GetValue();																				// Store image exposure time
			char real_filename[30];																												// Construct file name from given strings, exposure time, and image number
			strncpy(real_filename, "!", sizeof(real_filename));
			strcat(real_filename, "fitsimg_cam0_");
			char exp_str[6];
			sprintf(exp_str, "%d", exposure);
			strcat(real_filename, exp_str);
			strcat(real_filename, "_");
			char num_str[6];
			sprintf(num_str, "%d", i);
			strcat(real_filename, num_str);
			strcat(real_filename, ".fits");

			struct image *cam_image = new struct image; 																						// Construct image struct and set up parameters
			cam_image->imgGrab = ptrGrabResult1;
			cam_image->exposure = exposure;
			cam_image->temp = tempcam;
			cam_image->imgname = real_filename;
			cam_image->camname = newstr;

			if (write_basler_fits(cam_image) != 0)  																							// If image building from struct did not work
			{
				throw "Bad process in fits image writing!";																						// Throw an error
			}
			else 																																// If image building from struct did work
			{																																
				cout << "Image grab and write successful" << endl;																				// Print confirmation message
				delete(cam_image);																												// Free struct
			}
		}
		else  																																	// If image is not grabbed successfully, throw an error
		{
			cerr << "Error: " << ptrGrabResult1->GetErrorCode() << " " << ptrGrabResult1->GetErrorDescription() << endl;
			exitCode = 1;
		}
        if (ptrGrabResult2->GrabSucceeded())  																									// If image from camera 2 is grabbed successfully 
		{
			string file_name = (string) cameras[1].GetDeviceInfo().GetModelName() + " "+ (string) cameras[0].GetDeviceInfo().GetSerialNumber();	// Get device name from model name and serial number for first camera
			char* newstr = &file_name[0u];																										// Convert device name to char* for cfitsio
			int tempcam = (int)cameras[1].DeviceTemperature.GetValue();																			// Store device temperature
			int exposure = (int)cameras[1].ExposureTime.GetValue();																				// Store image exposure time
			char real_filename[30];																												// Construct file name from given strings, exposure time, and image number
			strncpy(real_filename, "!", sizeof(real_filename));
			strncpy(real_filename, "!", sizeof(real_filename));
			strcat(real_filename, "fitsimg_cam1_");
			char exp_str[6];
			sprintf(exp_str, "%d", exposure);
			strcat(real_filename, exp_str);
			strcat(real_filename, "_");
			char num_str[6];
			sprintf(num_str, "%d", i);
			strcat(real_filename, num_str);
			strcat(real_filename, ".fits");

			struct image *cam_image = new struct image; 																						// Construct image struct and set up parameters
			cam_image->imgGrab = ptrGrabResult2;
			cam_image->exposure = exposure;
			cam_image->temp = tempcam;
			cam_image->imgname = real_filename;
			cam_image->camname = newstr;

			if (write_basler_fits(cam_image) != 0)  																							// If image building from struct did not work
			{
				throw "Bad process in fits image writing!";																						// Throw an error
			}
			else 																																// If image building from struct did work
			{																																
				cout << "Image grab and write successful" << endl;																				// Print confirmation message
				delete(cam_image);																												// Free struct
			}
		}
		else  																																	// If image is not grabbed successfully, throw an error
		{
			cerr << "Error: " << ptrGrabResult2->GetErrorCode() << " " << ptrGrabResult2->GetErrorDescription() << endl;
			exitCode = 1;
		}
		++i;																																	// Increment image #
        }
    }
    catch (const GenericException &e)																											// Error handling
    {
        cerr << "An exception occurred." << endl
        << e.GetDescription() << endl;
        exitCode = 1;
    }
    PylonTerminate(); 																															// Releases all pylon resources. 
    return exitCode;
}


// Grab_MultipleCameras.cpp
/*
    Note: Before getting started, Basler recommends reading the Programmer's Guide topic
    in the pylon C++ API documentation that gets installed with pylon.
    If you are upgrading to a higher major version of pylon, Basler also
    strongly recommends reading the Migration topic in the pylon C++ API documentation.
    This sample illustrates how to grab and process images from multiple cameras
    using the CInstantCameraArray class. The CInstantCameraArray class represents
    an array of instant camera objects. It provides almost the same interface
    as the instant camera for grabbing.
    The main purpose of the CInstantCameraArray is to simplify waiting for images and
    camera events of multiple cameras in one thread. This is done by providing a single
    RetrieveResult method for all cameras in the array.
    Alternatively, the grabbing can be started using the internal grab loop threads
    of all cameras in the CInstantCameraArray. The grabbed images can then be processed by one or more
    image event handlers. Please note that this is not shown in this example.
*/
// Include files to use the PYLON API.
#include "write_basler_fits.h"
#include <pylon/usb/BaslerUsbInstantCameraArray.h>
// Limits the amount of cameras used for grabbing.
// It is important to manage the available bandwidth when grabbing with multiple cameras.
// This applies, for instance, if two GigE cameras are connected to the same network adapter via a switch.
// To manage the bandwidth, the GevSCPD interpacket delay parameter and the GevSCFTD transmission delay
// parameter can be set for each GigE camera device.
// The "Controlling Packet Transmission Timing with the Interpacket and Frame Transmission Delays on Basler GigE Vision Cameras"
// Application Notes (AW000649xx000)
// provide more information about this topic.
// The bandwidth used by a FireWire camera device can be limited by adjusting the packet size.
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
        if (ptrGrabResult1->GrabSucceeded())  						// If image is grabbed successfully 
		{
			string file_name = (string) cameras[0].GetDeviceInfo().GetModelName() + " "+ (string) cameras[0].GetDeviceInfo().GetSerialNumber();
			char* newstr = &file_name[0u];
			int tempcam = (int)cameras[0].DeviceTemperature.GetValue();
			int exposure = (int)cameras[0].ExposureTime.GetValue();
			char real_filename[30];							// Construct file name from exposure time
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

			struct image *cam_image = new struct image; 				// Construct image struct and set up parameters
			cam_image->imgGrab = ptrGrabResult1;
			cam_image->exposure = exposure;
			cam_image->temp = tempcam;
			cam_image->imgname = real_filename;
			cam_image->camname = newstr;

			if (write_basler_fits(cam_image) != 0)  				// If image building from struct did not work
			{
				throw "Bad process in fits image writing!";
			}
			else {									// If image building from struct did work
				cout << "Image grab and write successful" << endl;
				delete(cam_image);
			}
		}
		else  										// If image is not grabbed successfully, throw an error
		{
			cerr << "Error: " << ptrGrabResult1->GetErrorCode() << " " << ptrGrabResult1->GetErrorDescription() << endl;
			exitCode = 1;
		}
        if (ptrGrabResult2->GrabSucceeded())  						// If image is grabbed successfully 
		{
			string file_name = (string) cameras[1].GetDeviceInfo().GetModelName() + " "+ (string) cameras[0].GetDeviceInfo().GetSerialNumber();
			char* newstr = &file_name[0u];
			int tempcam = (int)cameras[1].DeviceTemperature.GetValue();
			int exposure = (int)cameras[1].ExposureTime.GetValue();
			char real_filename[30];							// Construct file name from exposure time
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

			struct image *cam_image = new struct image; 				// Construct image struct and set up parameters
			cam_image->imgGrab = ptrGrabResult2;
			cam_image->exposure = exposure;
			cam_image->temp = tempcam;
			cam_image->imgname = real_filename;
			cam_image->camname = newstr;

			if (write_basler_fits(cam_image) != 0)  				// If image building from struct did not work
			{
				throw "Bad process in fits image writing!";
			}
			else {									// If image building from struct did work
				cout << "Image grab and write successful" << endl;
				delete(cam_image);
			}
		}
		else  										// If image is not grabbed successfully, throw an error
		{
			cerr << "Error: " << ptrGrabResult2->GetErrorCode() << " " << ptrGrabResult2->GetErrorDescription() << endl;
			exitCode = 1;
		}
	++i;
        }
    }
    catch (const GenericException &e)
    {
        // Error handling
        cerr << "An exception occurred." << endl
        << e.GetDescription() << endl;
        exitCode = 1;
    }
    // Releases all pylon resources. 
    PylonTerminate(); 
    return exitCode;
}


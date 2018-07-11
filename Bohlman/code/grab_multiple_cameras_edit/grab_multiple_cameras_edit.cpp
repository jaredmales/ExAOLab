
/*! \file grab_multiple_cameras.cpp
\brief A file that repeatedly grabs images from 2 cameras connected to system
*/

// Include files to use the PYLON API.
#include "write_basler_fits.h"
#include <pylon/usb/BaslerUsbInstantCameraArray.h>

static const size_t c_maxCamerasToUse = 2;
int main(int argc, char* argv[])
{
    int exitCode = 0, exposure, width = 640, height = 480;
	uint32_t countOfImagesToGrab;
	if (argc == 3) {																						// If there are 2 command line arguments
		exposure = atoi(argv[1]);																			// Set given exposure, if it's invalid, report error and exit
		if (exposure == 0) 
		{
			fprintf(stderr, "ERROR: Invalid exposure value.\n");
			exit(1);
		}																														
		countOfImagesToGrab = atoi(argv[2]);																// Set given number of images, if it's invalid, report error and exit
		//if (exposure == 0) 
		if (countOfImagesToGrab == 0) 
		{
			fprintf(stderr, "ERROR: Invalid number of images value.\n");
			exit(1);
		}
	} else {																								// If there are not 3 command line arguments
		exposure = 1000;																					// Default value of exp: 1000 us
		countOfImagesToGrab = 10000;																		// Default number of images to grab: 1000
	}

    if (argc == 3) {																						// If there are 2 command line arguments
		exposure = atoi(argv[1]);																			// Set given exposure, if it's invalid, report error and exit
		if (exposure == 0) 
		{
			fprintf(stderr, "ERROR: Invalid exposure value.\n");
			exit(1);
		}																														
		countOfImagesToGrab = atoi(argv[2]);																// Set given number of images, if it's invalid, report error and exit
		//if (exposure == 0) 
		if (countOfImagesToGrab == 0) 
		{
			fprintf(stderr, "ERROR: Invalid number of images value.\n");
			exit(1);
		}
	} else {																								// If there are not 3 command line arguments
		exposure = 1000;																					// Default value of exp: 1000 us
		countOfImagesToGrab = 10000;																		// Default number of images to grab: 1000
	}
    // Before using any pylon methods, the pylon runtime must be initialized. 
    PylonInitialize();

    unsigned long *camera_array1;
    unsigned long *camera_array2;																			// Creates array of longs for mean image
	camera_array1 = (unsigned long*) calloc(width*height, sizeof(unsigned long));
	camera_array2 = (unsigned long*) calloc(width*height, sizeof(unsigned long));

	if (camera_array1 == NULL || camera_array2 == NULL) 																				// If it's unable to be allocated, report memory error and exit
	{
		fprintf(stderr, "ERROR: Memory for median image did not allocate correctly.\n");
		exit(1);
	}

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
        int expos = exposure;											//SET EXPOSURE HERE
        cameras.Open();
		cameras[0].ExposureAuto.SetValue(ExposureAuto_Off);
		cameras[0].ExposureTime.SetValue(expos);
		cameras[1].ExposureAuto.SetValue(ExposureAuto_Off);
		cameras[1].ExposureTime.SetValue(expos);
        // This smart pointer will receive the grab result data.
        CGrabResultPtr ptrGrabResult1;
		CGrabResultPtr ptrGrabResult2;
        // Grab c_countOfImagesToGrab from the cameras.
		int i = 0;
		int j, k;
		cameras.StartGrabbing();	

        for( uint32_t i = 0; i < countOfImagesToGrab && cameras.IsGrabbing(); ++i)
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
			uint8_t *pImageBuffer1 = (uint8_t *)ptrGrabResult1->GetBuffer();								// Retrieve image buffer

			for (k = 1; k <= height; k = k+1) {   														// Looks through each pixel in an image and adds to mean image array in position
				for (j = 1; j <= width; j = j+1) {
					camera_array1[((k-1)*width + (j-1))] = camera_array1[((k-1)*width + (j-1))] + pImageBuffer1[((k-1)*width + (j-1))];
				}
			}
		}
		else  																																	// If image is not grabbed successfully, throw an error
		{
			cerr << "Error: " << ptrGrabResult1->GetErrorCode() << " " << ptrGrabResult1->GetErrorDescription() << endl;
			exitCode = 1;
		}
        if (ptrGrabResult2->GrabSucceeded())  																									// If image from camera 2 is grabbed successfully 
		{
			uint8_t *pImageBuffer2 = (uint8_t *)ptrGrabResult2->GetBuffer();								// Retrieve image buffer

			for (k = 1; k <= height; k = k+1) {   														// Looks through each pixel in an image and adds to mean image array in position
				for (j = 1; j <= width; j = j+1) {
					camera_array2[((k-1)*width + (j-1))] = camera_array2[((k-1)*width + (j-1))] + pImageBuffer2[((k-1)*width + (j-1))];
				}
			}
		}
		else  																																	// If image is not grabbed successfully, throw an error
		{
			cerr << "Error: " << ptrGrabResult2->GetErrorCode() << " " << ptrGrabResult2->GetErrorDescription() << endl;
			exitCode = 1;
		}
		++i;																																	// Increment image #
        }

        for (k = 1; k <= height; k = k+1) {   																// Looks through each pixel in the image array and divides by amount of images taken to find mean
				for (j = 1; j <= width; j = j+1) {
					camera_array1[((k-1)*width + (j-1))] = camera_array1[((k-1)*width + (j-1))]/countOfImagesToGrab;
					camera_array2[((k-1)*width + (j-1))] = camera_array2[((k-1)*width + (j-1))]/countOfImagesToGrab;
				}
		}

		fitsfile *fptr1, *fptr2;																						// Create a fits file for the mean image, check for errors throughout creation process
		long naxes[2] = { width, height };
		long naxis = 2, fpixel = 1;

		if (fits_create_file(&fptr1, "!mean_image_1.fits", &exitCode) != 0) {
			exitCode = 1; 						
			fits_report_error(stderr, exitCode);
		}  														

		if (fits_create_img(fptr1, LONG_IMG, naxis, naxes, &exitCode) != 0) { 			
			exitCode = 1;			
			fits_report_error(stderr, exitCode);  														
		}

		if (fits_write_img(fptr1, TLONG, fpixel, width*height, camera_array1, &exitCode) != 0) { 	
			exitCode = 1;		
			fits_report_error(stderr, exitCode);  	
		}													

		if (fits_close_file(fptr1, &exitCode) != 0) {		
			exitCode = 1;											
			fits_report_error(stderr, exitCode);
		}  														

		free(camera_array1);
		std::cout << "Coadding image number 1 produced!" << endl; 														// Completion message

		if (fits_create_file(&fptr2, "!mean_image_2.fits", &exitCode) != 0) {
			exitCode = 1; 						
			fits_report_error(stderr, exitCode);
		}  														

		if (fits_create_img(fptr2, LONG_IMG, naxis, naxes, &exitCode) != 0) { 			
			exitCode = 1;			
			fits_report_error(stderr, exitCode);  														
		}

		if (fits_write_img(fptr2, TLONG, fpixel, width*height, camera_array2, &exitCode) != 0) { 	
			exitCode = 1;		
			fits_report_error(stderr, exitCode);  	
		}													

		if (fits_close_file(fptr2, &exitCode) != 0) {		
			exitCode = 1;											
			fits_report_error(stderr, exitCode);
		}  														

		free(camera_array2);
		std::cout << "Coadding image number 2 produced!" << endl; 														// Completion message	
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


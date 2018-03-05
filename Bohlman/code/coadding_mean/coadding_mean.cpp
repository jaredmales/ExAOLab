/*! \file coadding_mean.cpp
\brief A documented file that initializes the pylon resources, takes a given number of images at a given exposure, coadds images in memory, finds mean image pixel value, and writes mean to fits file.
*/

#include "write_basler_fits.h"

//  Main function
/** Initializes pylon resources, takes images, finds mean, writes mean, closes all pylon resources.
* \return an integer: 0 upon exit success, 1 otherwise
*/
int main(int argc, ///< [in] the integer value of the count of the command line arguments
	char* argv[] ///< [ch.ar] the integer value of the count of the command line arguments
)
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
	
	PylonInitialize(); 																						// Initializes pylon runtime before using any pylon methods
	
	unsigned long *camera_array;																			// Creates array of longs for mean image
	camera_array = (unsigned long*) calloc(width*height, sizeof(unsigned long));

	if (camera_array == NULL) 																				// If it's unable to be allocated, report memory error and exit
	{
		fprintf(stderr, "ERROR: Memory for median image did not allocate correctly.\n");
		exit(1);
	}

	try
	{
		int j, k;
		CDeviceInfo info;																					// Set up attached Basler USB camera
		info.SetDeviceClass(Camera_t::DeviceClass());
		CBaslerUsbInstantCamera camera( CTlFactory::GetInstance().CreateFirstDevice());
		camera.Open();																						// Opens camera parameters to grab images and set exposure time
		camera.ExposureAuto.SetValue(ExposureAuto_Off);														// Set exposure
		camera.ExposureTime.SetValue(exposure);
			
		camera.StartGrabbing(countOfImagesToGrab);															// Start grabbing a provided amount of images
		while (camera.IsGrabbing()) {																		// While the camera is still grabbing
			CGrabResultPtr ptrGrabResult;
			camera.RetrieveResult(5000, ptrGrabResult, TimeoutHandling_ThrowException);						// Grab image result with timeout of 5000 ms
			if (ptrGrabResult->GrabSucceeded())  															// If image is grabbed successfully: 
			{
				uint8_t *pImageBuffer = (uint8_t *)ptrGrabResult->GetBuffer();								// Retrieve image buffer

				for (k = 1; k <= height; k = k+1) {   														// Looks through each pixel in an image and adds to mean image array in position
					for (j = 1; j <= width; j = j+1) {
						camera_array[((k-1)*width + (j-1))] = camera_array[((k-1)*width + (j-1))] + pImageBuffer[((k-1)*width + (j-1))];
					}
				}
			} else  																						// If image is not grabbed successfully, throw an error
			{
				cerr << "Error: " << ptrGrabResult->GetErrorCode() << " " << ptrGrabResult->GetErrorDescription() << endl;
			}
		}
		camera.Close();																						// Close camera

		
		for (k = 1; k <= height; k = k+1) {   																// Looks through each pixel in the image array and divides by amount of images taken to find mean
				for (j = 1; j <= width; j = j+1) {
					camera_array[((k-1)*width + (j-1))] = camera_array[((k-1)*width + (j-1))]/countOfImagesToGrab;
				}
		}

		fitsfile *fptr;																						// Create a fits file for the mean image, check for errors throughout creation process
		long naxes[2] = { width, height };
		long naxis = 2, fpixel = 1;

		if (fits_create_file(&fptr, "!mean_image.fits", &exitCode) != 0) {
			exitCode = 1; 						
			fits_report_error(stderr, exitCode);
		}  														

		if (fits_create_img(fptr, LONG_IMG, naxis, naxes, &exitCode) != 0) { 			
			exitCode = 1;			
			fits_report_error(stderr, exitCode);  														
		}

		if (fits_write_img(fptr, TLONG, fpixel, width*height, camera_array, &exitCode) != 0) { 	
			exitCode = 1;		
			fits_report_error(stderr, exitCode);  	
		}													

		if (fits_close_file(fptr, &exitCode) != 0) {		
			exitCode = 1;											
			fits_report_error(stderr, exitCode);
		}  														

		free(camera_array);
		std::cout << "Coadding image produced!" << endl; 														// Completion message																										// Find median of all images through find_median()
	}
	catch (const GenericException &e)  																		// Provides Pylon error handling.
	{
		std::cerr << "An exception occurred." << endl
			<< e.GetDescription() << endl;
		exitCode = 1;
	}
	Pylon::PylonTerminate();   																				// Releases all pylon resources. 
	return exitCode;
}

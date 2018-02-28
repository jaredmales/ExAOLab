/*! \file construct_flat_field.cpp
\brief A documented file that initializes the pylon resources, takes a number of images at the lowest exposure, and finds/writes median image in rder to make an averaged flat image
*/

#include "write_basler_fits.h"

//  Main function
/** Initializes pylon resources, takes pictures, closes all pylon resources.
* \return an integer: 0 upon exit success, 1 otherwise
*/
int main(int argc, ///< [in] the integer value of the count of the command line arguments
	char* argv[] ///< [ch.ar] the integer value of the count of the command line arguments
)
{
	int exposure;
	uint32_t countOfImagesToGrab;
	if (argc == 3) {																															// if there are more than 2 command line arguments
		exposure = atoi(argv[1]);																														// exposure is first argument																														
		countOfImagesToGrab = atoi(argv[2]);																									// number of images to grab is second argument
	}
	else {																																	// if there are less than 2 command line arguments
		exposure = 1000;																															// default value of exp: 5000 us
		countOfImagesToGrab = 10000;																											// default number of images to grab: 10
	}	
	int exitCode = 0;
	PylonInitialize(); 
	int width = 640, height = 480; 																													// Initializes pylon runtime before using any pylon methods
	
	
	unsigned long *camera_array;
	camera_array = (unsigned long*) calloc(640*480, sizeof(unsigned long));
	

	//long *camera_array;
	//camera_array = (long*) calloc(640*480, sizeof(long));

	if (camera_array == NULL) {
		fprintf(stderr, "ERROR: Memory did not allocate correctly.\n");
		exit(1);
	}

	try
	{
		int i, j, k;
		CDeviceInfo info;																													// Set up attached Basler USB camera
		info.SetDeviceClass(Camera_t::DeviceClass());
		Camera_t camera(CTlFactory::GetInstance().CreateFirstDevice());   																	// Creates an instant camera object with the camera device found first.
		for (i = 0; i < countOfImagesToGrab; ++i)																						// For the desired amount of images
		{
			CGrabResultPtr ptrGrabResult;															
			
			camera.Open();																													// Opens camera parameters
			camera.ExposureAuto.SetValue(ExposureAuto_Off);
			camera.ExposureTime.SetValue(exposure);
			
			camera.StartGrabbing(1);  																										// Starts the grabbing of a singular image.
			camera.RetrieveResult(5000, ptrGrabResult, TimeoutHandling_ThrowException);  													// Waits for an image and then retrieves it. A timeout of 5000 ms is used.
			camera.Close();

			if (ptrGrabResult->GrabSucceeded())  																							// If image is grabbed successfully: 
			{
				uint8_t *pImageBuffer = (uint8_t *)ptrGrabResult->GetBuffer();

				for (k = 1; k <= 480; k = k+1) {   //Looks through each pixel in a picture
					for (j = 1; j <= 640; j = j+1) {
						//printf("%d\n", pImageBuffer[((k-1)*width + (j-1))]);
						camera_array[((k-1)*width + (j-1))] = camera_array[((k-1)*width + (j-1))] + pImageBuffer[((k-1)*width + (j-1))];
					}
				}
			}
			else  																															// If image is not grabbed successfully, throw an error
			{
				cerr << "Error: " << ptrGrabResult->GetErrorCode() << " " << ptrGrabResult->GetErrorDescription() << endl;
			}
		}
		for (k = 1; k <= 480; k = k+1) {   //Looks through each pixel in a picture
				for (j = 1; j <= 640; j = j+1) {
					camera_array[((k-1)*width + (j-1))] = camera_array[((k-1)*width + (j-1))]/countOfImagesToGrab;
					//printf("%d\n", camera_array[((k-1)*width + (j-1))]);
				}
		}
		fitsfile *fptr;																											// Create a fits file for the median image, check for errors throughout creation process
		long naxes[2] = { width, height };
		long naxis = 2, fpixel = 1;

		if (fits_create_file(&fptr, "!median_image.fits", &exitCode) != 0) {
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
		std::cout << "Median image produced!" << endl; 																// Completion message																										// Find median of all images through find_median()
	}
	catch (const GenericException &e)  																										// Provides Pylon error handling.
	{
		std::cerr << "An exception occurred." << endl
			<< e.GetDescription() << endl;
		exitCode = 1;
	}
	Pylon::PylonTerminate();   																												// Releases all pylon resources. 
	return exitCode;
}

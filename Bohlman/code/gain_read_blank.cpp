/*! \file gain_read_blank.cpp
\brief A documented file that 
Initializes the pylon resources, takes the photos,
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
	int expArray[c_countOfImagesToGrab];
	int i;
	for (i = 0; i < c_countOfImagesToGrab; i++)
		expArray[i] = 59;
	i = 0;
	std::ios_base::sync_with_stdio(false);  	// Seperates std from stdio
	PylonInitialize();  	// Initializes pylon runtime before using any pylon methods
	try
	{
		CBaslerUsbInstantCamera camera(CTlFactory::GetInstance().CreateFirstDevice());   // Creates an instant camera object with the camera device found first.
		string file_name = (string)camera.GetDeviceInfo().GetModelName() + " " + (string)camera.GetDeviceInfo().GetSerialNumber();
		char* newstr = &file_name[0u];
		camera.StartGrabbing(c_countOfImagesToGrab);  		// Starts the grabbing of c_countOfImagesToGrab images.
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
				cout << "Error: " << ptrGrabResult->GetErrorCode() << " " << ptrGrabResult->GetErrorDescription() << endl; //Prints an error
			}
			++i;
		}
	}
	catch (const GenericException &e)  // Provides error handling.
	{
		std::cerr << "An exception occurred." << endl
			<< e.GetDescription() << endl;
		exitCode = 1;
	}
	
	const char *names[c_countOfImagesToGrab];
	int kk = 0;
	for (kk; kk < c_countOfImagesToGrab; ++kk) {
		char base_filename[30];
		strncpy(base_filename, "fitsimg_exp59_", sizeof(base_filename));
		char num[3];
		sprintf(num, "%d", kk);
		strcat(base_filename, num);
		strcat(base_filename, ".fits");
		names[kk] = _strdup(base_filename);
	}

	int width = 640, height = 480;
	double *image_arr = (double*) calloc(width * height,sizeof(double));
	int j, k;
	for (k = 1; k <= height; ++k) {
		for (j = 1; j <= width; ++j) {
			int ii;
			double pixel_arr[c_countOfImagesToGrab];
			for (ii = 0; ii < c_countOfImagesToGrab; ++ii) {
				fitsfile *fptr;
				long fpixel[2] = { j,k };
				double pixels;
				if (fits_open_file(&fptr, names[ii], READONLY, &exitCode))
				{
					fits_report_error(stderr, exitCode);  // Prints out any fits error messages
					exit(1);
				}
				else
				{
					fits_read_pix(fptr, TDOUBLE, fpixel, 1, NULL, &pixels, NULL, &exitCode);
					pixel_arr[ii] = pixels;
				}
				fits_close_file(fptr, &exitCode);
			}
			std::vector<double> v(pixel_arr, pixel_arr + c_countOfImagesToGrab);
			std::nth_element(v.begin(), v.begin() + v.size() / 2, v.end());
			image_arr[(k-1)*width + (j - 1)] = v[v.size() / 2];
		}
		if (k%40 == 0)
			cout << "Processing images..." << endl;
	}

	fitsfile *fptr;
	long naxes[2] = { width, height };
	long naxis = 2, fpixel = 1;

	if (fits_create_file(&fptr, "!median_lowexp_image.fits", &exitCode) != 0) //Creates new fits file
		fits_report_error(stderr, exitCode);  // Prints out any fits error messages

	if (fits_create_img(fptr, LONGLONG_IMG, naxis, naxes, &exitCode) != 0)  //Creates the primary array image
		fits_report_error(stderr, exitCode);  // Prints out any fits error messages

	if (fits_write_img(fptr, TDOUBLE, fpixel, width*height, image_arr, &exitCode) != 0)  // Writes pointer values to the image
		fits_report_error(stderr, exitCode);  // Prints out any fits error messages

	if (fits_close_file(fptr, &exitCode) != 0) // Closes the fits file
		fits_report_error(stderr, exitCode);  // Prints out any fits error messages
	
	std::cout << "Median image at minimum exposure produced!" << endl;
	std::cerr << endl << "Press Enter to exit." << endl;
	while (cin.get() != '\n');
	Pylon::PylonTerminate();   // Releases all pylon resources. 
	return exitCode;
}

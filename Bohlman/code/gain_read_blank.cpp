/*Fixes to the problem of the program taking forever:
Don't open every file 640*480 times, have an array of fits pointers
Put values directly into vector

/*! \file gain_read_blank.cpp
\brief A documented file that 
Initializes the pylon resources, takes the photos, finds median photo.
*/

#include "stdafx.h"
#include "write_basler_fits.h"

//  Gets median image of the previous images.   
/** Gets the names of the images, and goes into a for loop which gets the median of each pixel for each image and applies it to an int array, which then writes to a fits file
*  \return an integer: 0 upon exit success, 1 otherwise
*/

	
int find_median() {
	int exitCode = 0;
	const char *names[c_countOfImagesToGrab];
	int kk = 0;
	for (kk; kk < c_countOfImagesToGrab; ++kk) {  //creates an array of names for each of the files previously made
		char base_filename[30];
		strncpy(base_filename, "fitsimg_exp59_", sizeof(base_filename));
		char num[3];
		sprintf(num, "%d", kk);
		strcat(base_filename, num);
		strcat(base_filename, ".fits");
		names[kk] = base_filename;
	}

	//Array of fits pointers;
	std::vector<fitsfile*> fpt_arr(c_countOfImagesToGrab);
	int cc;
	for (cc = 0; cc < c_countOfImagesToGrab; ++cc) {
		fitsfile* fptr;
		fpt_arr.push_back(fptr);
		fits_open_file(&(fpt_arr.at(cc)), names[cc], READONLY, &exitCode);
	}

	int width = 640, height = 480;
	double *image_arr = (double*)calloc(width * height, sizeof(double)); //Array of pixel values for final image
	int j, k;
	for (k = 1; k <= height; ++k) {   //Looks through each pixel in a picture
		for (j = 1; j <= width; ++j) {
			int ii;
			double pixel_arr[c_countOfImagesToGrab];
			for (ii = 0; ii < c_countOfImagesToGrab; ++ii) { //For each image
				fitsfile *fptr;
				long fpixel[2] = { j,k };
				double pixels;
				fits_read_pix(fpt_arr.at(ii), TDOUBLE, fpixel, 1, NULL, &pixels, NULL, &exitCode); //read a singular pixel
				pixel_arr[ii] = pixels;	//put it in an array to be compared with all other pixels
			}
			std::vector<double> v(pixel_arr, pixel_arr + c_countOfImagesToGrab);  //Find median of all pixel values
			std::nth_element(v.begin(), v.begin() + v.size() / 2, v.end());
			image_arr[(k - 1)*width + (j - 1)] = v[v.size() / 2];
		}
		if (k % 40 == 0) //progress messages
			cout << "Processing images..." << endl;
	}

	for (cc = 0; cc < c_countOfImagesToGrab; ++cc) {
		fits_close_file(fpt_arr.at(cc), &exitCode);
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

	free(image_arr);
	std::cout << "Median image at minimum exposure produced!" << endl; //We did it
	return exitCode;
}

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
				cam_image->imgname = real_filename;

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
		exitCode = find_median();
	}
	catch (const GenericException &e)  // Provides error handling.
	{
		std::cerr << "An exception occurred." << endl
			<< e.GetDescription() << endl;
		exitCode = 1;
	}
	std::cerr << endl << "Press Enter to exit." << endl;
	while (cin.get() != '\n');
	Pylon::PylonTerminate();   // Releases all pylon resources. 
	return exitCode;
}

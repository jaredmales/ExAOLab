/*! \file write_basler_fits.cpp
\brief A documented file that writes a fits file from a passed image struct.
Takes the struct and creates the fits file and updates the keys that provide image data.
*/

#include "write_basler_fits.h"


//  Writes fits file of the image struct passed to function.   
/** Gets the buffer of the image, parameters of image, generates a file name based off of exposure value, and creates image while error checking.
*  \return an integer: 0 upon exit success, 1 otherwise
*/
int write_basler_fits(struct image *cam_image  ///< [st] the struct of the image
)
{
	uint8_t *pImageBuffer = (uint8_t *)cam_image->imgGrab->GetBuffer();  // Gets image buffer from pointer to image data
	int exitCode = 0;
	fitsfile *fptr;       //Creates pointer to the FITS file; defined in fitsio.h   
	int width = (int)cam_image->imgGrab->GetWidth();
	int height = (int)cam_image->imgGrab->GetHeight();
	long  fpixel = 1, naxis = 2;
	long naxes[2] = { width, height };

	if (fits_create_file(&fptr, cam_image->imgname, &exitCode) != 0) //Creates new fits file
	{
		fits_report_error(stderr, exitCode);  // Prints out any fits error messages
		return 1;
	}
	if (fits_create_img(fptr, BYTE_IMG, naxis, naxes, &exitCode) != 0)  //Creates the primary array image
	{
		fits_report_error(stderr, exitCode);  // Prints out any fits error messages
		return 1;
	}
	if (fits_update_key(fptr, TINT, "EXPOSURE", &(cam_image->exposure), "Total Exposure Time in microseconds", &exitCode) != 0)  //Writes exprosure keyword of image from struct
	{
		fits_report_error(stderr, exitCode);  // Prints out any fits error messages
		return 1;
	}
	if (fits_update_key(fptr, TSTRING, "INSTRUME", cam_image->camname, "Model of camera followed by serial number", &exitCode) != 0)  //Writes camera name followed by serial number keyword from struct
	{
		fits_report_error(stderr, exitCode);  // Prints out any fits error messages
		return 1;
	}
	if (fits_update_key(fptr, TINT, "TEMP", &(cam_image->temp), "Temperature of camera in celsius when picture was taken", &exitCode) != 0)  //Writes camera temperature keyword from struct
	{
		fits_report_error(stderr, exitCode);  // Prints out any fits error messages
		return 1;
	}
	if (fits_write_date(fptr, &exitCode) != 0)
	{
		fits_report_error(stderr, exitCode);  // Prints out any fits error messages
		return 1;
	}

	if (fits_write_img(fptr, TBYTE, fpixel, width*height, pImageBuffer, &exitCode) != 0)  // Writes pointer values to the image
	{
		fits_report_error(stderr, exitCode);  // Prints out any fits error messages
		return 1;
	}
	if (fits_close_file(fptr, &exitCode) != 0) // Closes the fits file
	{
		fits_report_error(stderr, exitCode);  // Prints out any fits error messages
		return 1;
	}
	return 0;
}

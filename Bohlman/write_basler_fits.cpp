#include "stdafx.h"
#include "write_basler_fits.h"

//  Writes fits file of the image stored in memory   
/** Returns a 0 if all processes worked, 1 otherwise
*  \return an integer: 0 upon exit success, 1 otherwise
*/
int write_basler_fits(struct image *cam_image  ///< [st] the struct of the image with temperature, exposure time, image pointer, and camera model/serial number
)
{
#ifdef PYLON_WIN_BUILD
	uint8_t *pImageBuffer = (uint8_t *)cam_image->imgGrab->GetBuffer();  // Gets image buffer from pointer to image data
#endif		
	int exitCode = 0;
	fitsfile *fptr;       //Creates pointer to the FITS file; defined in fitsio.h   
	int width = (int)cam_image->imgGrab->GetWidth();
	int height = (int)cam_image->imgGrab->GetHeight();
	long  fpixel = 1, naxis = 2;
	long naxes[2] = { width, height };
	char real_filename[25];
	strncpy(real_filename, "!", sizeof(real_filename));
	strcat(real_filename, "fitsimg_exp");
	char exp_str[6];
	sprintf(exp_str, "%d", cam_image->exposure);
	strcat(real_filename, exp_str);
	strcat(real_filename, ".fits");

	if (fits_create_file(&fptr, real_filename, &exitCode) != 0) //Creates new fits file
	{
		fits_report_error(stderr, exitCode);  // Prints out any fits error messages
		return 1;
	}
	if (fits_create_img(fptr, BYTE_IMG, naxis, naxes, &exitCode) != 0)  //Creates the primary array image
	{
		fits_report_error(stderr, exitCode);  // Prints out any fits error messages
		return 1;
	}
	if (fits_update_key(fptr, TLONG, "EXPOSURE", &(cam_image->exposure), "Total Exposure Time", &exitCode) != 0)  //Writes exprosure keyword of image from struct
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
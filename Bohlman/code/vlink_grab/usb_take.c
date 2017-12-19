// File: usb_grab.c
// Purpose: Uses the VLink F4 grabber to grab data from a phantom channel and output it to a fits file
// gcc usb_take.c -I /opt/EDTpdv -L/opt/EDTpdv -lpdv -lpthread -lm -ldl -L/opt/cfitsio -lcfitsio -lcurl -g -o ./usb_take

#include "edtinc.h"
#include "fitsio.h"

struct image {
	int exposure;
	int temp;
	int width;
	int height;
	unsigned char *imgGrab;
	char camname[50];
	char imgname[50];
};

int write_fits(struct image *cam_image  ///< [st] the struct of the image
)
{
	uint8_t *pImageBuffer = (uint8_t *)cam_image->imgGrab;  // Gets image buffer from pointer to image data
	int exitCode = 0;
	fitsfile *fptr;       //Creates pointer to the FITS file; defined in fitsio.h   
	long  fpixel = 1, naxis = 2;
	long naxes[2] = { cam_image->width, cam_image->height };

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
	if (fits_write_img(fptr, TBYTE, fpixel, cam_image->width*cam_image->height, pImageBuffer, &exitCode) != 0)  // Writes pointer values to the image
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

int main ( int argc, char *argv[] ) {
	PdvDev *pdv_p = pdv_open_channel(NULL, 0, 2);
	int	exposure = 5000;
	struct image *cam_image = (struct image*) malloc(sizeof(struct image));;
	pdv_set_exposure(pdv_p, exposure);
	int height = pdv_get_height(pdv_p);
	int width = pdv_get_width(pdv_p);
	pdv_multibuf(pdv_p, 4) ;
	unsigned char *image = pdv_image(pdv_p);
	cam_image->exposure = exposure;
	cam_image->temp = 0;
	cam_image->width = width;
	cam_image->height = height;
	cam_image->imgGrab = image;
	char str1[]= "VisionLink F4 Board Channel 2";
	char str2[]= "!test.fits";
	strncpy(cam_image->camname,str1,sizeof(cam_image->camname));
	strncpy(cam_image->imgname,str2,sizeof(cam_image->imgname));
	write_fits(cam_image);
	int t, last_timeouts = 0;
	t = pdv_timeouts(pdv_p);
	if (t > last_timeouts) {
		printf("got timeout\n");
    		// add recovery code here -- see simple_take.c for example
    		// last_timeouts = t;
        }
	pdv_close(pdv_p) ;
	return 0;
}

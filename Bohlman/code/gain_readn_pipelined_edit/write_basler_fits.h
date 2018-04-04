/*! \file write_basler_fits.h
\brief A documented header file that contains resources necessary for Pylon functions and cfitsio functions
Includes all libraries necessary to run the program, sets up namespaces, creates struct for image data, and 
contains prototype so other cpp files can use write_basler_fits() function
*/

#include <pylon/PylonIncludes.h>
#ifdef PYLON_WIN_BUILD
#    include <pylon/PylonGUI.h>
#endif
#include <pylon/PixelData.h>
#include <pylon/GrabResultData.h>
#include <pylon/usb/BaslerUsbInstantCamera.h>
#include <pylon/usb/_BaslerUsbCameraParams.h>
#include <GenApi/IFloat.h>
#include <iostream>
#include <algorithm>
#include <iostream>
#include <vector>
#include <algorithm>
#include <functional>
#include "fitsio.h"


typedef Pylon::CBaslerUsbInstantCamera Camera_t;
using namespace Basler_UsbCameraParams;

using namespace Pylon;
using namespace std;
static const uint32_t c_countOfImagesToGrab = 90;
static const uint32_t a_countOfImagesToGrab = 100;

/*! \struct image
//struct that is passed to write_basler_fits() that has data for the image
* holds the int value for exposure, int value for temperature of camera, pointer to image, and string for camera model name/serial number
*/
struct image {
	int exposure;
	int temp;
	CGrabResultPtr imgGrab;
	char* camname;
	char* imgname;
};

extern int write_basler_fits(struct image *cam_image);

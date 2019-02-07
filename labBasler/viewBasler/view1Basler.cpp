/*! \file view1Basler.cpp
\brief A documented file that takes command line arguments for exposure and number of images to grab and outputs fits files
Initializes the pylon resources, takes the photos, and passes a struct to write_basler_fits() function with image data in it
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

#define DS9INTERFACE_NO_EIGEN
#include "../lib/mx/improc/ds9Interface.hpp"

typedef Pylon::CBaslerUsbInstantCamera Camera_t;
using namespace Basler_UsbCameraParams;

using namespace Pylon;
using namespace std;

//  Main function
/** Initializes pylon resources, takes pictures, closes all pylon resources.
* \return an integer: 0 upon exit success, 1 otherwise
*/
int main( int argc, ///< [in] the integer value of the count of the command line arguments
          char* argv[] ///< [ch.ar] the integer value of the count of the command line arguments
        )
{
   
   typedef int16_t pixelT;
   
   int exitCode = 0;
   int exposure = 0;
    
   if (argc == 2) 
   {                                // if there are more than 2 command line arguments
      exposure = atoi(argv[1]);     // exposure is first argument
   }
   else 
   {  // if there are less than 2 command line arguments set default
      exposure = 1000;  // default value of exp: 5000 us
   }
   
   PylonInitialize(); // Initializes pylon runtime before using any pylon methods
   
   try
   {
      CDeviceInfo info; // Set up attached Basler USB camera
      info.SetDeviceClass(Camera_t::DeviceClass());
      CBaslerUsbInstantCamera camera( CTlFactory::GetInstance().CreateFirstDevice());
      string file_name = (string)camera.GetDeviceInfo().GetModelName() + "_" + (string)camera.GetDeviceInfo().GetSerialNumber(); // Gets camera model name and serial number
      std::cerr << file_name << std::endl;
      
      mx::improc::ds9Interface ds9(file_name);
      ds9.connect();
      
      camera.RegisterConfiguration( new CAcquireContinuousConfiguration , RegistrationMode_ReplaceAll, Cleanup_Delete);
      
      camera.Open(); // Opens camera parameters to grab images and set exposure time
      camera.ExposureAuto.SetValue(ExposureAuto_Off); // Set exposure
      camera.ExposureTime.SetValue(exposure);
      camera.PixelFormat.SetValue(PixelFormat_Mono10);
      
      camera.StartGrabbing(GrabStrategy_LatestImageOnly ); // Start grabbing a provided amount of images
   
      CGrabResultPtr ptrGrabResult;
      
     
   
      while (camera.IsGrabbing()) 
      {
         CGrabResultPtr ptrGrabResult;
      
         //int tempcam = (int)camera.DeviceTemperature.GetValue(); // Gets and stores temperature of camera
         camera.RetrieveResult(5000, ptrGrabResult, TimeoutHandling_ThrowException);  // Waits for an image and then retrieves it. A timeout of 5000 ms is used
         
         if (ptrGrabResult->GrabSucceeded()) // If image is grabbed successfully 
         {
            if( ds9.display((pixelT *) ptrGrabResult->GetBuffer(), mx::improc::getFitsBITPIX<pixelT>(), sizeof(pixelT), 640, 480, 1, 1) < 0)
            {
               break;
            }
         }
         else  // If image is not grabbed successfully, throw an error
         {
            cerr << "Error: " << ptrGrabResult->GetErrorCode() << " " << ptrGrabResult->GetErrorDescription() << endl;
            exitCode = 1;
         }
      }
      camera.Close();
   }
   catch (const GenericException &e) // Provides Basler error handling.
   {
      cerr << "An exception occurred." << endl
         << e.GetDescription() << endl;
      exitCode = 1;
   }
   PylonTerminate();   // Releases all pylon resources. 
   return exitCode;
}

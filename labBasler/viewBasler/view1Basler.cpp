/*! \file view1Basler.cpp
\brief A documented file that takes command line arguments for exposure and number of images to grab and outputs fits files
Initializes the pylon resources, takes the photos, and passes a struct to write_basler_fits() function with image data in it
*/

#include <signal.h>
#include <iostream>
#include <algorithm>
#include <iostream>
#include <vector>
#include <algorithm>
#include <functional>


#include <pylon/PylonIncludes.h>
#ifdef PYLON_WIN_BUILD
#    include <pylon/PylonGUI.h>
#endif
#include <pylon/PixelData.h>
#include <pylon/GrabResultData.h>
#include <pylon/usb/BaslerUsbInstantCamera.h>
#include <pylon/usb/_BaslerUsbCameraParams.h>
#include <GenApi/IFloat.h>

#define DS9INTERFACE_NO_EIGEN
#include "../lib/mx/improc/ds9Interface.hpp"

typedef Pylon::CBaslerUsbInstantCamera Camera_t;
using namespace Basler_UsbCameraParams;

using namespace Pylon;
using namespace std;



bool timeToDie = false;

void sigTermHandler( int signum,
                     siginfo_t *siginf,
                     void *ucont
                   )
{
   std::cerr << "Exiting . . . \n";
   //Suppress those warnings . . .
   static_cast<void>(signum);
   static_cast<void>(siginf);
   static_cast<void>(ucont);
  
   timeToDie = true;
}



//  Main function
/** Initializes pylon resources, takes pictures, closes all pylon resources.
* \return an integer: 0 upon exit success, 1 otherwise
*/
int main( int argc, ///< [in] the integer value of the count of the command line arguments
          char* argv[] ///< [ch.ar] the integer value of the count of the command line arguments
        )
{
   std::string argv0 = argv[0];
   
   /* First install signal handling */
   struct sigaction act;
   sigset_t set;

   act.sa_sigaction = sigTermHandler;
   act.sa_flags = SA_SIGINFO;
   sigemptyset(&set);
   act.sa_mask = set;

   errno = 0;
   if( sigaction(SIGTERM, &act, 0) < 0 )
   {
      std::cerr << " (" << argv0 << "): error setting SIGTERM handler: " << strerror(errno) << "\n";
      return -1;
   }
   
   
   /* Now process args */
   typedef int16_t pixelT;
   
   int exitCode = 0;
   int exposure = 1000;
   
   std::string serNum; // Serial number of camera to connect to
   std::string camName;
   
   if (argc == 2) 
   {                                // if there are more than 2 command line arguments
      exposure = atoi(argv[1]);     // exposure is first argument
   }   
   else if (argc == 3) 
   {                                
      serNum = argv[1]; 
      exposure = atoi(argv[2]);
   }
   else if (argc == 4)
   {
      serNum = argv[1]; 
      exposure = atoi(argv[2]);
      camName = argv[3];
   }

   
   PylonInitialize(); // Initializes pylon runtime before using any pylon methods
   
   try
   {
      CDeviceInfo info; // Set up attached Basler USB camera
      info.SetDeviceClass(Camera_t::DeviceClass());
      
      if(serNum != "") info.SetSerialNumber(serNum.c_str());
      CBaslerUsbInstantCamera camera( CTlFactory::GetInstance().CreateFirstDevice(info));
      
      std::cerr << "Found camera: " << (string)camera.GetDeviceInfo().GetModelName() << "\n";
      std::cerr << "Serial number: " << (string)camera.GetDeviceInfo().GetSerialNumber() << "\n";
      
      if(camName == "")
      {
         camName = (string)camera.GetDeviceInfo().GetModelName() + "_" + (string)camera.GetDeviceInfo().GetSerialNumber();
      }
      
      std::cerr << "Camera name: " << camName << "\n";
      
      mx::improc::ds9Interface ds9(camName);
      ds9.connect();
      
      camera.RegisterConfiguration( new CAcquireContinuousConfiguration , RegistrationMode_ReplaceAll, Cleanup_Delete);
      
      camera.Open(); // Opens camera parameters to grab images and set exposure time
      camera.PixelFormat.SetValue(PixelFormat_Mono10); //Make it 10 bit
      
      camera.ExposureAuto.SetValue(ExposureAuto_Off); // Set exposure
      camera.ExposureTime.SetValue(exposure);


      
      camera.StartGrabbing(GrabStrategy_LatestImageOnly ); // Start grabbing a provided amount of images
   
      CGrabResultPtr ptrGrabResult;
      
      while (camera.IsGrabbing() && !timeToDie) 
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
            exitCode = -1;
            break;
         }
      }
      camera.Close();
   }
   catch (const GenericException &e) // Provides Basler error handling.
   {
      cerr << "An exception occurred." << endl
         << e.GetDescription() << endl;
      exitCode = -1;
   }
   
   PylonTerminate();   // Releases all pylon resources. 
   
   return exitCode;
}

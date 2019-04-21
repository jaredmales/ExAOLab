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

#include "write_basler_fits.h"

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
   
   int numImages = 0;
   int exposure = 1000;
   
   std::string serNum; // Serial number of camera to connect to
   std::string camName;
   
   if(argc == 2)
   {
      numImages = atoi(argv[1]);
   }
   else if (argc == 3) 
   {                               
      numImages = atoi(argv[1]);
      exposure = atoi(argv[2]);  
   }   
   else if (argc == 4) 
   {
      numImages = atoi(argv[1]);
      serNum = argv[2]; 
      exposure = atoi(argv[3]);
   }
   else if (argc == 5)
   {
      numImages = atoi(argv[1]);
      serNum = argv[2]; 
      exposure = atoi(argv[3]);
      camName = argv[4];
   }
   else
   {
      std::cerr << "Wrong number of command line arguments.\n";
      std::cerr << "Must specify number of exposures...\n";
      
      return -1;
   }
   
   std::string model;

   PylonInitialize(); // Initializes pylon runtime before using any pylon methods
   
   try
   {
      CDeviceInfo info; // Set up attached Basler USB camera
      info.SetDeviceClass(Camera_t::DeviceClass());
      
      if(serNum != "") info.SetSerialNumber(serNum.c_str());
      CBaslerUsbInstantCamera camera( CTlFactory::GetInstance().CreateFirstDevice(info));
      
      model = (string)camera.GetDeviceInfo().GetModelName();
      serNum = (string)camera.GetDeviceInfo().GetSerialNumber();
      std::cerr << "Found camera: " <<  model << "\n";
      std::cerr << "Serial number: " <<  serNum << "\n";
      
      if(camName == "")
      {
         camName = model + "_" + serNum;
      }
      
      std::cerr << "Camera name: " << camName << "\n";
            
      camera.RegisterConfiguration( new CAcquireContinuousConfiguration , RegistrationMode_ReplaceAll, Cleanup_Delete);
      
      camera.Open(); // Opens camera parameters to grab images and set exposure time
      camera.PixelFormat.SetValue(PixelFormat_Mono10); //Make it 10 bit
      
      camera.ExposureAuto.SetValue(ExposureAuto_Off); // Set exposure
      camera.ExposureTime.SetValue(exposure);


      
      camera.StartGrabbing(GrabStrategy_LatestImageOnly ); // Start grabbing a provided amount of images
   
      CGrabResultPtr ptrGrabResult;
      
      image cam_image; //Chris's image structure for writing.
      
      int j = 0;
      while (camera.IsGrabbing() && !timeToDie && j < numImages) 
      {
         CGrabResultPtr ptrGrabResult;
      
         int tempcam = (int)camera.DeviceTemperature.GetValue(); // Gets and stores temperature of camera
         camera.RetrieveResult(5000, ptrGrabResult, TimeoutHandling_ThrowException);  // Waits for an image and then retrieves it. A timeout of 5000 ms is used
         
         if (ptrGrabResult->GrabSucceeded()) // If image is grabbed successfully 
         {
            
            cam_image.imgGrab = ptrGrabResult;
            cam_image.exposure = exposure;
            cam_image.temp = tempcam;
               
            cam_image.model = (char *) model.c_str();
            cam_image.serial = (char *) serNum.c_str();
            cam_image.camname = (char *) camName.c_str();
               
            char real_filename[256];// Construct file name from given strings, exposure time, and image number
            
            strncpy(real_filename, "!", sizeof(real_filename));
            strcat(real_filename, camName.c_str());
            strcat(real_filename, "_");
            char exp_str[64];
            snprintf(exp_str, sizeof(exp_str), "%d", cam_image.exposure);
            strcat(real_filename, exp_str);
            char num_str[64];
            snprintf(num_str, sizeof(num_str), "_%d", j);
            strcat(real_filename, num_str);
            strcat(real_filename, ".fits");
            
            cam_image.imgname = real_filename;

            if (write_basler_fits(&cam_image) != 0) // If image building from struct did not work
            {
               throw "Bad process in fits image writing!";// Throws error
            }
            
            j++;
         }
         else  // If image is not grabbed successfully, throw an error
         {
            cerr << "Error: " << ptrGrabResult->GetErrorCode() << " " << ptrGrabResult->GetErrorDescription() << endl;
            exitCode = -1;
            break;
         }
      }
      
      std::cerr << "Wrote " << j << "/" << numImages << " images\n";
      
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

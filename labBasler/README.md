
# Dependencies

## Basler Pylon SDK
   1) Download the Basler Pylon SDK, which is available for free on baslerweb.com

   2) Unpack it.

   3) Follow the instructions in the INSTALL to install to `/opt`.

## libxpa:
```
git clone https://github.com/ericmandel/xpa.git
cd xpa
./configure
make
sudo make install
```

## cfitsio:
You should be able to install this from a package manager.  Be sure to get the `devel` version.  If not, build from source like so:
```
wget http://heasarc.gsfc.nasa.gov/FTP/software/fitsio/c/cfitsio3450.tar.gz
tar -xvzf cfitsio3450.tar.gz
cd cfitsio
./configure --prefix=/usr/local
make
sudo make install
```
Note: change the version of the package to the latest one if you want.

## saoimage ds9.  
Currently (April 2019) ds9 version 7.5 works, but versions  >= 8 don't seem to work with libxpa anymore.

# viewBasler
## Build
 Once the above installs are done,
```
$ cd viewBasler
$ make
```

## Running view1Basler

The `view1Basler` program takes 0 to 3 arguments.  If no arguments are specified, it opens the first Basler camera it finds and sets an exposure time of 1000 micro-seconds.  `ds9` will open and begin displaying the images.

If just 1 argument is given, it is treated as an expsoure time in micro-seconds.
```
$ ./view1Basler
```

If 2 arguments are given, the first will be treated as the serial number of the camera to connect to, the second as exposure time in micro-seconds.
```
$ ./view1Basler 22411493 10000
```

If 3 arguments are give, the first is the serial number, the second is exposure time in micro-seconds, and the third is the name of the camera.
```
$ ./view1Basler  22411493 10000 cam1
```

# writeBasler
## Build
 Once the above installs are done,
```
$ cd writeBasler
$ make
```

## Running write1Basler

The `write1Basler` program takes 1 to 4 arguments.  The first argument is always the number of images to write.

If just 1 argument is given, it is the number of images to write.
```
$ ./write1Basler 10
```
will write 10 images.

If 2 arguments are given, the first is the number of images to write, the second is the expsoure time in micro-seconds.
```
$ ./write1Basler 10 10000
```
will write 10 images with an exposure time of 10,000 micro-seconds.

If 3 arguments are given, the first is the number of images to write, the second will be treated as the serial number of the camera to connect to, the third as exposure time in micro-seconds.
```
$ ./write1Basler 10 22411493 10000
```
will write 10 images from camera 22411493 with an exposure time of 10,000 micro-seconds.

If 4 arguments are give, the first is the number of images to write, the second will be treated as the serial number of the camera to connect to, the third as exposure time in micro-seconds, and the fourth is the name of the camera.
```
$ ./write1Basler 10 22411493 10000 cam1
```
will write 10 images from camera 22411493 with an exposure time of 10,000 micro-seconds, using `cam1` as the camera name.

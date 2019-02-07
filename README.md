# [PassPort Photo app](https://passport-photo.azurewebsites.net)
<!-- Travis-CI Build Status: [![Build Status](https://travis-ci.org/dpar39/ppp.svg?branch=master)](https://travis-ci.org/dpar39/ppp) -->

AppVeyor Build Status: [![Build Status](https://ci.appveyor.com/api/projects/status/github/dpar39/ppp?svg=true)](https://ci.appveyor.com/project/dpar39/ppp)

This app allows users to create passport photos with automatic picture size and rotation cropped to comply with the standards accepted in most countries. Users get a tiled photo in your favorite print format (e.g. 4"x6" or 5"x7") with the appropriate resolution ready for printing.

The main motivation for this app is how expensive passport photo services can be in many countries. For instance, OfficeWorks in Australia currently charges [$14.95](http://www.officeworks.com.au/print/print-and-copy/print-services/passport-and-id-photo) for four passport photos. Walgreens near my home charged me [$13.99](https://photo.walgreens.com/walgreens/pdpsdpitems/type=passport_photos) for only two a few weeks ago. Obviously, not all photos taken at home with a smartphone camera will have the quality to meet the standards, but if you are on the budget and/or have the time to ensure you capture a picture of yourself with good resolution, proper focus and low noise, then this app will do the maths to prepare an output tiled photo ready to be printed for less than a dollar at your closest photo center.

<div style="text-align:center"><img src ="research/model/operation-principle.png"/></div>

# Installation
The software can be built and run in either Windows or Unix based operating systems.

## Dependencies (platform independent)
- Python 2.7 or higher
- CMake 3.5.1 or higher
- node.js 8.x

#### Windows-specific dependencies
- Visual Studio 2015 or 2017

#### Linux-specific dependencies
- gcc 4.8 or above (C++11 compiler)

## Compiling and running
At the moment, the web app is still under development, you can try it out [here](https://passport-photo.azurewebsites.net). Also the node.js addon that does the heavy lifting can be used to generate passport photos from the command line:

- Clone this repository somewhere in your computer. Then run scripts _build.linux64.sh_ or _build.win64.bat_ depending on your platform. This will take a while because third party libraries such as _OpenCV_, _Poco_ and _GMock_ are built from source code. If everything goes well the installation directory (e.g. install_release_x64) should contain the addon module (_addon.node_) and a script _test.js_.
- Edit _test.js_ and set the path of the input picture you would like to process. Feel free to edit the printing definition to your needs. By default, it is configured as per the [US Passport requirements](https://travel.state.gov/content/passports/en/passports/photos/photos.html) (2" x 2" with face length of 1.1875" and eyes to picture bottom distance of 1.25"). The output size is 4" x 6" with 300dpi resolution.
- Run a terminal and change directory to your install directory, then run _node_ _test.js_ and your print-ready photo will be created.

## Approach taken

In order to crop and scale the face of the person to a particular passport requirement, the following approach was investigated. Given the set of detected face landmarks *A*, *B*, *C* and *D*, we would like to estimate *P* and *Q* with an accuracy that is sufficient to ensure that the face in the output photo fall within the limits of the measure requirements. In other words, the estimated location of the crown (*P’*) and chin point (*Q’*) should be such that the distance *P’Q’* scaled by the distance between the ideal location of the crown (*P*) and chin point (*Q*) falls within the tolerance range allowed in the passport requirement. For the Australian passport requirements, the allowed scale range is **±5.88%** for a face heigth between 32 and 36mm: [(36mm - 32mm)/(36mm + 32mm)]

To develop and validate the proposed approach, facial landmarks from the [SCFace database](http://www.scface.org/) were used. The SCFace database contains images for 130 different subjects and frontal images of each individual were carefully annotated by the [Biometric Recognition Group - ATVS at Escuela Politecnica Superior of the Universidad Autonoma de Madrid [ATVS]](https://atvs.ii.uam.es/scfacedb_landmarks.html).
The procedure to estimate *P’* and *Q’* from *A*, *B*, *C* and *D* is as follow: Firstly, points *M* and *N* are found as the center of segments *AB* and *CD* respectively. *P’* and *Q’* are expected to fall in the line that passes through *M* and *N*. Then using a normalization distance *KK = |AB| + |MN|* and scale constants *α* and *β*, we estimate *P’Q’* = *αKK* and *M’Q’* = *βKK*. From the dataset *α* and *β* were adjusted to minimise the estimation error of *P'* and *Q'*.

<div style="text-align:center"><img src ="research/model/key-facial-landmarks.png"/></div>


## Building the Android App
### Debian based OS

sudo add-apt-repository ppa:webupd8team/java
sudo apt-get update
sudo apt-get install oracle-java8-installer
sudo apt install oracle-java8-set-default

sudo apt-get install libpcap0.8-dev

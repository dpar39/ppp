# Passport photo app  [![Build Status](https://travis-ci.org/dpar39/ppp.svg?branch=master)](https://travis-ci.org/dpar39/ppp)

Recruiters please read [here](https://github.com/dpar39/ppp/wiki/Recruiters-README) for information on the skills demonstrated in this project.

This app *WILL* allow to create passport photo prints with automatic picture cropping and rotation based on the standard accepted in most countries. You'll get a tiled photo in your favorite format (e.g. 4"x6" or 5"x7") with the appropriate resolution ready to print.

# Installation
The software can be built and run in either Windows or Unix based operating systems.

## Dependencies (OS Indepent)
- Python 2.7 or higher
- CMake 3.0 or higher

### Windows-only dependencies
- Visual Studio 2012, 2013 or 2015
- In Windows, Node.js is built from source and the C++ addon is built and linked with CMake

### Linux-only
- Node.js 4.x
- Node-gyp (needed to compile the C++ addon)

## Compiling the code
Clone this repo somewhere in your computer. Then run scripts _build.linux64.sh_ or _build.win64.bat_ depending on your platform. This will take a while because third party libraries such as _OpenCV_, _Poco_ and _GMock_ are built from source code.
If everything goes well the install directory (e.g. install_release_x64) should contain the addon module (_addon.node_) and a _test.js_ that can be edited for testing.

Edit _test.js_ and set the path of the input picture you would like to process. Feel free to edit the printing definition to your needs. At the moment it is configured for the US Passport standard (2"x2" with face height to
Run a terminal and change directory to your install directory


# Usage
At the moment, the web app is still under construction, but the Node.js addon that does the heavy lifting can alredy be used to generate passport photos from the command line... 

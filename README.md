# [Photo ID Creator](https://myphotoidapp.firebaseapp.com/) - A tool to prepare photo IDs
<!-- Travis-CI Build Status: [![Build Status](https://travis-ci.org/dpar39/ppp.svg?branch=master)](https://travis-ci.org/dpar39/ppp) -->

<!--  [![Build Status](https://ci.appveyor.com/api/projects/status/github/dpar39/ppp?svg=true)](https://ci.appveyor.com/project/dpar39/ppp) -->

This app allows users to create passport photos with automatic picture size and rotation cropped to comply with the standards accepted in most countries. Users get a tiled photo in your favorite print format (e.g. 4"x6" or 5"x7") with the appropriate resolution ready for printing.

The main motivation for this app is how expensive passport photo services can be in many countries. For instance, OfficeWorks in Australia currently charges [$16.95](https://www.officeworks.com.au/print/print-and-copy/instore-services/passport-and-id-photo) for four passport photos.

Walgreens near my home charged me [$14.99](https://photo.walgreens.com/walgreens/pdpsdpitems/type=passport_photos) for only two a few weeks ago. Obviously, not all photos taken at home with a smartphone camera will have the quality to meet photo ID requirements, but if you are on the budget and/or have the time to ensure you capture a picture of yourself with good resolution, proper focus and illumination, then this app will do the maths to prepare an output tiled photo ready to be printed for less than a dollar at your closest photo center.

<div style="text-align:center"><img src ="research/model/operation-principle.png"/></div>

## Algorithm in a nutshell

In order to crop and scale the face of the person to a particular passport requirement, the following approach was investigated. Given the set of detected face landmarks *A*, *B*, *C* and *D*, we would like to estimate *P* and *Q* with an accuracy that is sufficient to ensure that the face in the output photo fall within the limits of the size requirements. In other words, the estimated location of the crown (*P’*) and chin point (*Q’*) should be such that the distance *P’Q’* scaled by the distance between the ideal location of the crown (*P*) and chin point (*Q*) falls within the tolerance range allowed in the passport specifications. For instance, in the case of the Australian passport, the allowed scale range is **±5.88%** for a face height between 32 and 36mm: [(36mm - 32mm)/(36mm + 32mm)]

To develop and validate the proposed approach, facial landmarks from the [SCFace database](http://www.scface.org/) were used. The SCFace database contains images for 130 different subjects and frontal images of each individual were carefully annotated by the [Biometric Recognition Group - ATVS at Escuela Politecnica Superior of the Universidad Autonoma de Madrid [ATVS]](https://atvs.ii.uam.es/scfacedb_landmarks.html).
The procedure to estimate *P’* and *Q’* from *A*, *B*, *C* and *D* is as follow: Firstly, points *M* and *N* are found as the center of segments *AB* and *CD* respectively. *P’* and *Q’* are expected to fall in the line that passes through *M* and *N*. Then using a normalization distance *KK = |AB| + |MN|* and scale constants *α* and *β*, we estimate *P’Q’* = *αKK* and *M’Q’* = *βKK*. From the dataset *α* and *β* were adjusted to minimize the estimation error of *P'* and *Q'*.

<div style="text-align:center"><img src ="research/model/key-facial-landmarks.png"/></div>

### Update

In a second iteration, the algorithm to detect facial landmarks has been substituted by a face alignment algorithm proposed by Vahid Kazemi 2014: [One Millisecond Face Alignment with an Ensemble of Regression Trees](http://www.csc.kth.se/~vahidk/face_ert.html). An implementation of the algorithm is available in [dlib C++ library](http://dlib.net/). This library ships with a model capable of detecting 68 landmarks, but has a compressed size of ~ 61MB. This is large enough to become a problem when packing an Android application to be deployed at Google Play store or to serve it on a web application that does client side processing (like we do with WebAssembly). Because of that, I have retrained the model to detect only 28 landmarks of the original set. This reduced the model size to 16.9MB when compressed, which can still be considered large to serve on a web application, but not prohibitive.

## How to run it

This application is currently provided in two forms: As a command line utility or as a [Firebase web app](https://myphotoidapp.firebaseapp.com/) fully running on the browser (i.e. backend only serves static files). The web app can also be built and run on Android using [Ionic capacitor](https://capacitor.ionicframework.com/), but this is still not well supported. The software is fully cross-platform, written in C++ and TypeScript, and has been built on Windows 10 with Visual Studio 2015 and 2017, Linux Mint with GCC 8.x and Mac with Clang 7.0. Non C++17 compliant compilers can also be used, but boost/filesystem is needed to build and run C++ unit tests.

### Prerequisites you'll need to install yourself

- CMake 3.12 or higher
- Python 2.7+ (build scripts)

#### If you are building the command line application

- A C++17 compliant compiler: On Windows use Microsoft Visual C++ 2017. On Mac LLVM 7.x. On Linux GCC 8.x

#### If you are building the web application only

- node.js 10.x and NPM

### List of libraries, frameworks and tools used under the hood

#### Build tools and IDEs

- CMake and ninja
- Emscripten SDK
- Clang Format
- VSCode with:
  - Prettier
  - Clang-Format
  - C/C++ Intellisense
  - Angular2-inline
  - Code Spell checker
  - Bracket Pair Colorizer

#### Core libraries (C++/WebAssembly)

- OpenCV 4.0
- DLib 19.6
- Google test / Google mock (v1.8.x)

#### Web Application

- Angular 7.x / Bootstrap 4.x / Interact.js, etc.

#### Android App

- Android SDK
- Android Studio
- Ionic capacitor

### Building the command line application and run C++ unit tests

```batch
python build.py --test -a x64
```

### Building the web application

```batch
python build.py --test -a wasm web
```

### Building the Android application

```batch
python build.py -a wasm web android
```

### Generating a Visual Studio project/solutions for C++ development

```batch
python build.py --gen_vs_sln -bdebug
```

## Building the Android App on a Debian based Linux OS

sudo add-apt-repository ppa:webupd8team/java
sudo apt-get update
sudo apt-get install oracle-java8-installer
sudo apt install oracle-java8-set-default
sudo apt-get install libpcap0.8-dev

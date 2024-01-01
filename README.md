# [Photo ID Creator](https://myphotoidapp.firebaseapp.com/) - A tool to prepare photo IDs

<p align="center">
  <a href="https://github.com/dpar39/ppp/actions/workflows/ci.yaml"  title="workflow" style="display:inline-block;">
    <img src="https://github.com/dpar39/ppp/actions/workflows/ci.yaml/badge.svg" alt="build status"/>
  </a>
  <a href="https://www.buymeacoffee.com/dpar39" title="Buy me a coffee" style="display: inline-block; margin-left:10px">
    <img src="https://img.shields.io/badge/Buy%20me%20a%20coffee-$5-blue?logo=buy-me-a-coffee&style=flat" alt="Buy me a coffee"/>
  </a>
</p>
This project implements a web-based application to create photographs in compliance with many ID forms and documents such as passports, visas, licenses, etc. The application takes as input a photograph of a person's face and automatically applies resizing and rotation to comply with the specified photo ID requirement. Then, the user can generate a tiled photo for a given print format such as 4x6" or 5x7" and have it printed at home or at your nearest photo center.

<div style="text-align:center"><img src ="docs/workflow.png"/></div>

## History of the project

I started this project several years ago with the dream of covering the gaps of existing web-based and Android/iOS apps that can produce valid photos for IDs and official applications. Being an immigrant, firstly in Australia, then in the USA, meant that I often found myself needing passport-sized photos for myself and my family members. Of course, this is a service that is offered in many places, but it is never cheap, especially for people starting from the ground up in a new country, possibly without a job, or with a large family. Traditional photo ID services also have the inconvenience of a trip to your nearest location. This is usually not a big deal, but what about parents with babies or people with disability or mobility problems?

Over the years, the tech stack I've used in this project has changed significantly, but the goal has always been the same: _provide_ a free client-side application that people can use to prepare their own photographs. Firstly, I always wanted it to be a tool where personal data (i.e. the user's photographs) never leave their PC or phone, unless they explicitly decide to print it online or share the result electronically. Secondly, I want the workflow to be _so_ simple that if you are capable of taking a picture of someone's face with your phone and know what type of document you need (e.g. a US Passport), the app should take care of the rest with just a couple of clicks.

Obviously, not all photos taken at home with a smartphone camera will have the quality to meet photo ID requirements, but if you are on the budget and/or have the time to ensure you capture a picture of yourself with good resolution, proper focus and illumination, then this app will do the maths to prepare an output tiled photo ready to be printed for much less than a dollar at your closest photo center.

## Tech stack of the application

The application is made out out two components, a C++ library compiled to WebAssembly and a front-end application developed using Ionic with Angular backend in TypeScript.

Image processing algorithms of the application are developed in modern C++ and compiled to WebAssembly with Emscripten. The C++ code lives inside folder `libppp` and is built using [Bazel](https://bazel.build/). Originally, all C++ code was building with [CMake](https://cmake.org/), but since the app now uses [MediaPipe](https://developers.google.com/mediapipe/solutions), it only made sense to move the build system to Bazel given the complexities of porting MediaPipe to a CMake-based build system.

Libraries used in C++ code:

- OpenCV (core, imgproc, imgcodecs, objdetect, etc.)
- MediaPipe and its dependencies (TensorFlow Lite)
- Google Test

The front-end of the application is built using

- Ionic Framework with Angular
- Interact.js (for gesture handling)
- ngx-color-picker (color picker component)
- flag-icons (icons of country flags)

## Getting started with the code

You'll need a POSIX system with Docker and bash available (WSL2, Linux or Mac). See section [Installing WSL2 on Windows](#Installing-WLS2-on-Windows) if you need help setting it up.

Then, either you install NPM or run the following by opening the folder in VSCode using DevContainers.

```bash
# build the webassembly package
npm run wasm:all

# Start the app in development mode
cd webapp && npm install && npm start
```

### Installing WLS2 on Windows

To setup WSL2 with docker, run the following steps in a Powershell or CMD as Administrator.

```batch
:: Check for wsl updates
wsl --update

:: Install Ubuntu 22.04
wsl --install Ubuntu-22.04
```

Now inside WSL install docker and exit:

```bash
echo $(whoami) ALL=\(root\) NOPASSWD:ALL | sudo tee /etc/sudoers.d/$(whoami)
printf '[boot]\nsystemd=true\n' | sudo tee /etc/wsl.conf
sudo apt-get update
sudo apt-get install apt-transport-https ca-certificates curl software-properties-common
curl -fsSL https://download.docker.com/linux/ubuntu/gpg | sudo apt-key add -
sudo add-apt-repository "deb [arch=amd64] https://download.docker.com/linux/ubuntu $(lsb_release -cs) stable"
sudo apt-get update
sudo apt-get install docker-ce -y
sudo usermod -aG docker $USER
sudo service docker start
sudo reboot
```

If everything worked well, then you should be able to run in CMD or Powershell:

```bash
wsl -d Ubuntu-22.04 docker run hello-world
```

If you want to do Typescript/Javascript development directly on Windows, install Node.js and NPM on Windows. With Powershell this can be achieved via `winget install OpenJS.NodeJS.LTS --accept-source-agreements`.

## Cropping to photo standard dimentions

In order to crop and scale the face of the person to a particular passport requirement, the following approach was investigated. Given the set of detected facial landmarks *A*, *B*, *C* and *D*, we would like to estimate *P* and *Q* with accuracy that is sufficient to ensure that the face in the output photo fall within the limits of the size requirements. In other words, the estimated location of the crown (*P’*) and chin point (*Q’*) should be such that the distance *P’Q’* scaled by the distance between the ideal location of the crown (*P*) and chin point (*Q*) falls within the tolerance range allowed in photo ID specifications. For instance, for the case of Australian passport, the allowed scale drift range is **±5.88%** given that the face height (chin to crown) must be between 32mm and 36mm: $\dfrac{1}{2} \times \dfrac{36 - 32}{0.5 \times (32+36)} = \pm 5.88$%.

<div style="text-align:center"><img src ="docs/key-facial-landmarks.png"/></div>

To develop and validate the proposed approach, facial landmarks from the [SCFace database](http://www.scface.org/) were used. The SCFace database contains images for 130 different subjects and frontal images of each individual were carefully annotated by the [Biometric Recognition Group - ATVS at Escuela Politecnica Superior of the Universidad Autonoma de Madrid [ATVS]](https://atvs.ii.uam.es/scfacedb_landmarks.html).
The procedure to estimate *P’* and *Q’* from *A*, *B*, *C* and *D* is as follow: Firstly, points *M* and *N* are found as the center of segments *AB* and *CD* respectively. *P’* and *Q’* are expected to fall in the line that passes through *M* and *N*. Then using a normalization distance *KK = |AB| + |MN|* and scale constants *α* and *β*, we estimate *P’Q’* = *αKK* and *M’Q’* = *βKK*. From the dataset *α* and *β* were adjusted to minimize the estimation error of *P'* and *Q'*.

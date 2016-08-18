#!/bin/bash

# install GTest and GMock
sudo apt-get install -qq libgtest-dev
cd /usr/src/gtest && sudo cmake . && sudo cmake --build . && sudo mv libg* /usr/local/lib/ ; cd -
sudo apt-get -qq install google-mock
cd /usr/src/gmock && sudo cmake . && sudo cmake --build . && sudo mv libg* /usr/local/lib/ ; cd -

# Install lcov
sudo apt-get install -qq lcov

# Install POCO and OPENCV
echo Installing Poco Libraries
sudo apt-get install -qq libpoco-dev
sudo apt-get install -qq libopencv-core-dev libopencv-imgproc-dev libopencv-objdetect-dev libopencv-highgui-dev

cmake 
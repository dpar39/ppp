#pragma once

// DLL export 
#ifdef WIN32
#ifdef DLLEXPORT
#define DECLSPEC __declspec(dllexport)
#else
#define DECLSPEC 
#endif
#else
#define DECLSPEC
#endif

#include <vector>

typedef unsigned char byte;
class PppEngine;

/*!@brief Wrapper class for this lib.
The purpose of this library is to decouple boost and opencv from the node add-on !*/

class DECLSPEC PublicPppEngine
{
public:

    PublicPppEngine();

    ~PublicPppEngine();

    void configure(const std::string &jsonConfig) const;

    /*!@brief Stores the image for processing 
    *  param[in] bufferData Pointer to the image data
    *  returns Image Id that can be used to recognise the image
    !*/
    std::string setImage(const char *bufferData, size_t bufferLength) const;
    
    std::string detectLandmarks(const std::string &imageId) const;
 
    /*!@brief Creates a tiled print from input image, crown/chin points and passport/canvas definition
    *  Output definition is passed as a JSON string with the following format:
    .{
    .    "preprocessing": {
    .    },
    .    "outputCanvas": {
    .       "height": 6,
    .       "width": 4,
    .       "resolution": 330,
    .       "border": 0.1,
    .       "units": "inch"
    .    },
    .    "passportDefinition": {
    .       "picWidth": 35,
    .       "picHeight": 45,
    .       "faceHeight": 34,
    .       "units": "mm"
    .    },
    .    "crownPoint": {
    .       "x": 500,
    .       "y": 10
    .    },
    .    "chinPoint":{
    .       "x": 500,
    .       "y": 600
    .    },
    .    "asBase64": true|falseS
    .}
    !*/
    std::string createTiledPrint(const std::string& imageId, const std::string &request) const;

private:
    PppEngine* m_pPppEngine;

private:
    static void setPngResolutionDpi(std::vector<byte>& imageStream, double resolution_ppmm);
};
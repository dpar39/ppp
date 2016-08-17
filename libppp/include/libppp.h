#pragma once

// DLL export 
#ifdef WIN32
#ifdef DLLEXPORT
#define DECLSPEC __declspec(dllexport)
#else
#define DECLSPEC __declspec(dllimport)
#endif
#else
#define DECLSPEC
#endif

#include <memory>
#include <string>
#include <ios>
#include <vector>

typedef unsigned char byte;
class PppEngine;

/*!@brief Wrapper class for this lib.
The purpose of this library is to decouple boost and opencv from the node add-on !*/

class DECLSPEC PublicPppEngine
{
public:

    PublicPppEngine();

    void configure(const std::string &jsonConfig);

    void setImage(const char *bufferData, int bufferLength);
    
    std::string detectLandmarks(const std::string &imageId);
 
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
    .    }
    .}
    !*/
    void createTiledPrint(const std::string &options, std::vector<byte> &pictureData);
    
private:
    std::shared_ptr<PppEngine> m_pPppEngine;
};
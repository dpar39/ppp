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

#include <opencv2/core/mat.hpp>

#include <string>
#include <vector>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/bind.h>
#endif

namespace ppp {
class PppEngine;

/*!@brief Wrapper class for this lib.
The purpose of this library is to decouple boost and opencv from the node add-on !*/

class DECLSPEC PublicPppEngine
{
public:
    PublicPppEngine();

    PublicPppEngine(const PublicPppEngine &) = delete;
    PublicPppEngine & operator=(const PublicPppEngine &) = delete;
    PublicPppEngine(PublicPppEngine &&) = delete;
    PublicPppEngine & operator=(PublicPppEngine &&) = delete;

    bool configure(const std::string & jsonConfig) const;

    bool isConfigured() const;

    /*!@brief Stores the image for processing
    *  param[in] bufferData Pointer to the image data
    *  param[in] bufferLength Length of the image data (if 0 or negative we assume it is base64 string)
    *  returns imageKey that can be used to retrieve the image
    !*/
    std::string setImageFromBuffer(const char * bufferData, size_t bufferLength) const;

    /**
     * \brief Retrieves the image as a PNG byte array
     * \param imageKey Image key used to retrieve the image from the store
     * \return The image data encoded as a PNG byte array
     */
    std::string getImageAsPngDataUrl(const std::string & imageKey) const;

    cv::Mat getImage(const std::string & imageKey) const;

    std::string detectLandmarks(const std::string & imageId) const;

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
    .    "asBase64": true|false
    .}
    !*/
    std::string createTiledPrint(const std::string & imageId, const std::string & request) const;

    std::string checkCompliance(const std::string & request) const;

#ifdef __EMSCRIPTEN__
    std::string setImage(const emscripten::val & typedArray) const;
#endif

private:
    PppEngine * m_pPppEngine;
};
} // namespace ppp

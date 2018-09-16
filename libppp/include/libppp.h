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
#include <string>

using BYTE = uint8_t;
class PppEngine;

/*!@brief Wrapper class for this lib.
The purpose of this library is to decouple boost and opencv from the node add-on !*/

class DECLSPEC PublicPppEngine
{
public:

    PublicPppEngine();

    ~PublicPppEngine();

    bool configure(const char *jsonConfig) const;

    /*!@brief Stores the image for processing
    *  param[in] bufferData Pointer to the image data
    *  param[in] bufferLength Length of the image data (if 0 or negative we assume it is base64 string)
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
    .    "asBase64": true|false
    .}
    !*/
    std::string createTiledPrint(const std::string& imageId, const std::string &request) const;

private:
    PppEngine* m_pPppEngine;

private:
    static void setPngResolutionDpi(std::vector<BYTE>& imageStream, double resolution_ppmm);
};

extern "C"
{
    bool set_image(const char *img_buf, int img_buf_size, char *img_id);

    bool configure(const char *config_json);

    bool detect_landmarks(const char *img_id, char *landmarks);

    bool create_tiled_print(const char *img_id, const char *request, unsigned char *out_buf, int &out_size);

    void get_last_error(char *err_message);
}
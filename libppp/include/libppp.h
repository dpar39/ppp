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

#include <string>
#include <vector>

namespace ppp
{
class PppEngine;

/*!@brief Wrapper class for this lib.
The purpose of this library is to decouple boost and opencv from the node add-on !*/

class DECLSPEC PublicPppEngine final
{
public:
    PublicPppEngine();
    ~PublicPppEngine();

    PublicPppEngine(const PublicPppEngine &) = delete;
    PublicPppEngine & operator=(const PublicPppEngine &) = delete;
    PublicPppEngine(PublicPppEngine &&) = delete;
    PublicPppEngine & operator=(PublicPppEngine &&) = delete;

    bool configure(const char * jsonConfig) const;

    bool isConfigured() const;

    /*!@brief Stores the image for processing
    *  param[in] bufferData Pointer to the image data
    *  param[in] bufferLength Length of the image data (if 0 or negative we assume it is base64 string)
    *  returns imageKey that can be used to retrieve the image
    !*/
    std::string setImage(const char * bufferData, size_t bufferLength) const;

    /**
     * \brief Retrieves the image as a PNG byte array
     * \param imageKey Image key used to retrieve the image from the store
     * \return The image data encoded as a PNG byte array
     */
    std::string getImage(const std::string & imageKey) const;

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

private:
    PppEngine * m_pPppEngine;
};
} // namespace ppp

extern "C"
{
    bool set_image(const char * img_buf, int img_buf_size, char * img_metadata);

    int get_image(const char * img_id, char * out_buf);

    bool configure(const char * config_json);

    bool is_configured();

    bool detect_landmarks(const char * img_id, char * landmarks);

    int create_tiled_print(const char * img_id, const char * request, char * out_buf);
}

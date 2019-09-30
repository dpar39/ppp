//
//
#include "libppp.h"
#include "CanvasDefinition.h"
#include "EasyExif.h"
#include "ImageStore.h"
#include "LandMarks.h"
#include "PhotoStandard.h"
#include "PppEngine.h"
#include "Utilities.h"

#include <opencv2/imgcodecs.hpp>
#include <regex>

#ifdef EMSCRIPTEN
#include <emscripten.h>
#else
#define EMSCRIPTEN_KEEPALIVE
#endif

using namespace std;

PublicPppEngine g_c_pppInstance;
string g_last_error;

cv::Point fromJson(rapidjson::Value & v)
{
    return cv::Point(v["x"].GetInt(), v["y"].GetInt());
}

PublicPppEngine::PublicPppEngine()
: m_pPppEngine(new PppEngine)
{
}

PublicPppEngine::~PublicPppEngine()
{
    delete m_pPppEngine;
}

bool PublicPppEngine::configure(const char * jsonConfig) const
{

    return m_pPppEngine->configure(jsonConfig);
}

std::string PublicPppEngine::setImage(const char * bufferData, const size_t bufferLength) const
{
    const auto & imageStore = m_pPppEngine->getImageStore();
    const auto imageKey = imageStore->setImage(bufferData, bufferLength);

    using namespace rapidjson;
    Document d;
    d.SetObject();
    auto & alloc = d.GetAllocator();

    d.AddMember("imgKey", imageKey, alloc);

    const auto exifInfo = imageStore->getExifInfo(imageKey);
    if (exifInfo)
    {
        d.AddMember("EXIFInfo", exifInfo->populate(alloc), alloc);
    }

    return Utilities::serializeJson(d);
}

std::string PublicPppEngine::getImage(const std::string & imageKey) const
{
    const auto & imageStore = m_pPppEngine->getImageStore();
    if (!imageStore->containsImage(imageKey))
    {
        return "";
    }
    const auto & image = imageStore->getImage(imageKey);
    return Utilities::encodeImageAsPng(image, false);
}

std::string PublicPppEngine::detectLandmarks(const std::string & imageId) const
{
    LandMarks landMarks;
    m_pPppEngine->detectLandMarks(imageId, landMarks);
    return landMarks.toJson();
}

std::string PublicPppEngine::createTiledPrint(const std::string & imageId, const std::string & request) const
{
    rapidjson::Document d;
    d.Parse(request.c_str());

    const auto ps = PhotoStandard::fromJson(d["standard"]);
    const auto canvas = CanvasDefinition::fromJson(d["canvas"]);
    auto crownPoint = fromJson(d["crownPoint"]);
    auto chinPoint = fromJson(d["chinPoint"]);
    auto asBase64Encode = false;

    if (d.HasMember("asBase64"))
    {
        asBase64Encode = d["asBase64"].GetBool();
    }

    const auto result = m_pPppEngine->createTiledPrint(imageId, *ps, *canvas, crownPoint, chinPoint);
    return Utilities::encodeImageAsPng(result, asBase64Encode, canvas->resolution_ppmm());
}

#pragma region C Interface
#define TRYRUN(statements)                                                                                             \
    try                                                                                                                \
    {                                                                                                                  \
        statements;                                                                                                    \
        std::cout << "Method '" << __FUNCTION__ << "' called successfully" << std::endl;                               \
        return true;                                                                                                   \
    }                                                                                                                  \
    catch (const std::exception & ex)                                                                                  \
    {                                                                                                                  \
        std::cout << "Method '" << __FUNCTION__ << "' failed: " << ex.what() << std::endl;                             \
        g_last_error = ex.what();                                                                                      \
        return false;                                                                                                  \
    }

EMSCRIPTEN_KEEPALIVE
bool set_image(const char * img_buf, int img_buf_size, char * img_metadata)
{
    TRYRUN(auto imgMetadata = g_c_pppInstance.setImage(img_buf, img_buf_size);
           strcpy(img_metadata, imgMetadata.c_str()););
}

EMSCRIPTEN_KEEPALIVE
bool configure(const char * config_json)
{
    TRYRUN(g_c_pppInstance.configure(config_json););
}

EMSCRIPTEN_KEEPALIVE
bool detect_landmarks(const char * img_id, char * landmarks)
{
    TRYRUN(auto landmarksStr = g_c_pppInstance.detectLandmarks(img_id); strcpy(landmarks, landmarksStr.c_str()););
}

EMSCRIPTEN_KEEPALIVE
int create_tiled_print(const char * img_id, const char * request, char * out_buf)
{
    try
    {
        auto output = g_c_pppInstance.createTiledPrint(img_id, request);
        const auto out_size = static_cast<int>(output.size());
        copy(output.begin(), output.end(), out_buf);
        return out_size;
    }
    catch (const std::exception & ex)
    {
        g_last_error = ex.what();
        return 0;
    }
}

EMSCRIPTEN_KEEPALIVE
int get_image(const char * img_id, char * out_buf)
{
    try
    {
        auto output = g_c_pppInstance.getImage(img_id);
        const auto out_size = static_cast<int>(output.size());
        copy(output.begin(), output.end(), out_buf);
        return out_size;
    }
    catch (const std::exception & ex)
    {
        g_last_error = ex.what();
        return 0;
    }
}

#pragma endregion

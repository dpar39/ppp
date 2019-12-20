//
//
#include "libppp.h"
#include "EasyExif.h"
#include "ImageStore.h"
#include "LandMarks.h"
#include "PhotoStandard.h"
#include "PppEngine.h"
#include "PrintDefinition.h"
#include "Utilities.h"

#include <opencv2/imgcodecs.hpp>
#include <regex>

#ifdef EMSCRIPTEN
#include <emscripten.h>
#else
#define EMSCRIPTEN_KEEPALIVE
#endif

using namespace std;
namespace ppp
{

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

bool PublicPppEngine::configure(const char * jsonConfig, void * callback) const
{
    return m_pPppEngine->configure(jsonConfig, callback);
}

bool PublicPppEngine::isConfigured() const
{
    return m_pPppEngine->isConfigured();
}

std::string PublicPppEngine::setImage(const char * bufferData, const size_t bufferLength) const
{
    const auto & imageStore = m_pPppEngine->getImageStore();
    const auto imageKey = imageStore->setImage(bufferData, bufferLength);

    using namespace rapidjson;
    Document d;
    d.SetObject();
    auto & alloc = d.GetAllocator();

    d.AddMember(StringRef(IMAGE_ID), imageKey, alloc);

    const auto exifInfo = imageStore->getExifInfo(imageKey);
    if (exifInfo)
    {
        d.AddMember(StringRef(EXIF_INFO), exifInfo->populate(alloc), alloc);
    }

    return Utilities::serializeJson(d, false);
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
    const auto & imageStore = m_pPppEngine->getImageStore();
    if (!imageStore->containsImage(imageId))
    {
        return "";
    }
    m_pPppEngine->detectLandMarks(imageId);
    const auto & landMarks = imageStore->getLandMarks(imageId);
    return landMarks->toJson(false);
}

std::string PublicPppEngine::createTiledPrint(const std::string & imageId, const std::string & request) const
{
    rapidjson::Document d;
    d.Parse(request.c_str());

    const auto ps = PhotoStandard::fromJson(d[PHOTO_STANDARD]);
    const auto canvas = PrintDefinition::fromJson(d[PRINT_DEFINITION]);
    auto crownPoint = fromJson(d[CROWN_POINT]);
    auto chinPoint = fromJson(d[CHIN_POINT]);
    auto asBase64Encode = false;

    if (d.HasMember(AS_BASE64))
    {
        asBase64Encode = d[AS_BASE64].GetBool();
    }

    const auto result = m_pPppEngine->createTiledPrint(imageId, *ps, *canvas, crownPoint, chinPoint);
    return Utilities::encodeImageAsPng(result, asBase64Encode, canvas->resolutionDpi());
}

std::string PublicPppEngine::checkCompliance(const std::string & request) const
{
    rapidjson::Document d;
    d.Parse(request.c_str());

    const std::string imageId = d[IMAGE_ID].GetString();
    const auto ps = PhotoStandard::fromJson(d[PHOTO_STANDARD]);
    const auto crownPoint = fromJson(d[CROWN_POINT]);
    const auto chinPoint = fromJson(d[CHIN_POINT]);

    vector<std::string> complianceCheckNames;

    const auto array = d[COMPLIANCE_CHECKS].GetArray();
    for (rapidjson::SizeType i = 0; i < array.Size(); i++)
    {
        complianceCheckNames.emplace_back(array[i].GetString());
    }

    return m_pPppEngine->checkCompliance(imageId, ps, crownPoint, chinPoint, complianceCheckNames);
}
} // namespace ppp

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
    using namespace ppp;
    TRYRUN(auto imgMetadata = g_c_pppInstance.setImage(img_buf, img_buf_size);
           strcpy(img_metadata, imgMetadata.c_str()););
}

EMSCRIPTEN_KEEPALIVE
bool configure(const char * config_json, int callback)
{
    using namespace ppp;
    TRYRUN(g_c_pppInstance.configure(config_json, (void *) callback););
}

EMSCRIPTEN_KEEPALIVE
bool is_configured()
{
    using namespace ppp;
    return g_c_pppInstance.isConfigured();
}

EMSCRIPTEN_KEEPALIVE
bool detect_landmarks(const char * img_id, char * landmarks)
{
    using namespace ppp;
    TRYRUN(auto landmarksStr = g_c_pppInstance.detectLandmarks(img_id); strcpy(landmarks, landmarksStr.c_str()););
}

EMSCRIPTEN_KEEPALIVE
int create_tiled_print(const char * img_id, const char * request, char * out_buf)
{
    using namespace ppp;
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
    using namespace ppp;
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

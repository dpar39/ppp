#include "PublicPppEngine.h"
#include "EasyExif.h"
#include "ImageStore.h"
#include "LandMarks.h"
#include "PhotoStandard.h"
#include "PppEngine.h"
#include "PrintDefinition.h"
#include "Utilities.h"

#include <glog/logging.h>

#include <opencv2/core/mat.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

#include <regex>

using namespace std;

FWD_DECL(RawBuffer)
class RawBuffer
{
private:
    char * m_data { nullptr };
    size_t m_size { 0 };

public:
    char * data() const
    {
        return m_data;
    }
    size_t size() const
    {
        return m_size;
    }

    explicit RawBuffer(size_t size)
    : m_size(size)
    {
        m_data = new char[m_size];
    }

    RawBuffer(const RawBuffer & rb)
    : m_size(rb.m_size)
    {
        m_data = new char[m_size];
        memcpy(m_data, rb.m_data, rb.m_size);
    }

    RawBuffer(RawBuffer && rb) noexcept
    : m_data(rb.m_data)
    , m_size(rb.m_size)
    {
        rb.m_data = nullptr;
        rb.m_size = 0;
    }

    RawBuffer & operator=(const RawBuffer & rb)
    {
        if (&rb == this) {
            return *this;
        }
        if (m_size != rb.m_size) {
            m_size = rb.m_size;
            delete[] m_data;
            m_data = new char[m_size];
        }
        memcpy(m_data, rb.m_data, rb.m_size);
        return *this;
    }

    RawBuffer & operator=(RawBuffer && rb) noexcept
    {
        delete[] m_data;
        m_size = rb.m_size;
        m_data = rb.m_data;
        rb.m_data = nullptr;
        rb.m_size = 0;
        return *this;
    }

    ~RawBuffer()
    {
        delete[] m_data;
    }
};

namespace ppp {

cv::Point fromJson(rapidjson::Value & v)
{
    return { v["x"].GetInt(), v["y"].GetInt() };
}

PublicPppEngine::PublicPppEngine()
: m_pPppEngine(new PppEngine)
{
    LOG(INFO) << "Constructing public PppEngine ...";
}

bool PublicPppEngine::configure(const std::string & jsonConfig) const
{
    LOG(INFO) << "Configuring engine ...";
    return m_pPppEngine->configure(jsonConfig);
}

bool PublicPppEngine::isConfigured() const
{
    return m_pPppEngine->isConfigured();
}

std::string PublicPppEngine::setImageFromBuffer(const char * bufferData, const size_t bufferLength) const
{
    const auto & imageStore = m_pPppEngine->getImageStore();
    const auto imageKey = imageStore->setImage(bufferData, bufferLength);

    using namespace rapidjson;
    Document d;
    d.SetObject();
    auto & alloc = d.GetAllocator();

    d.AddMember(StringRef(IMAGE_ID), imageKey, alloc);

    const auto exifInfo = imageStore->getExifInfo(imageKey);
    if (exifInfo) {
        d.AddMember(StringRef(EXIF_INFO), exifInfo->populate(alloc), alloc);
    }

    return Utilities::serializeJson(d, false);
}

std::string PublicPppEngine::getImageAsPngDataUrl(const std::string & imageKey) const
{
    const auto & imageStore = m_pPppEngine->getImageStore();
    if (!imageStore->containsImage(imageKey)) {
        LOG(ERROR) << "Could not find image with key = " << imageKey;
        return {};
    }
    const auto & image = getImage(imageKey);
    return Utilities::encodeImageAsPng(image, false);
}

cv::Mat PublicPppEngine::getImage(const std::string & imageKey) const
{
    cv::Mat rgba;
    const auto & imageStore = m_pPppEngine->getImageStore();
    if (!imageStore->containsImage(imageKey)) {
        LOG(ERROR) << "Could not find image with key = " << imageKey;
        return {};
    }
    cv::cvtColor(imageStore->getImage(imageKey), rgba, cv::COLOR_BGR2RGBA);
    return rgba;
}

std::string PublicPppEngine::detectLandmarks(const std::string & imageId) const
{
    LOG(INFO) << "Detecting facial landmarks ...";
    const auto & imageStore = m_pPppEngine->getImageStore();
    if (!imageStore->containsImage(imageId)) {
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

    if (d.HasMember(AS_BASE64)) {
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
    for (rapidjson::SizeType i = 0; i < array.Size(); i++) {
        complianceCheckNames.emplace_back(array[i].GetString());
    }

    return m_pPppEngine->checkCompliance(imageId, ps, crownPoint, chinPoint, complianceCheckNames);
}

#ifdef __EMSCRIPTEN__
using namespace emscripten;

RawBuffer toRawBuffer(const val & typedArray)
{
    const auto length = typedArray["byteLength"].as<unsigned int>();
    val heap = val::module_property("HEAPU8");
    val memory = heap["buffer"];
    RawBuffer rawBuffer(length);
    val memoryView = val::global("Uint8Array").new_(memory, reinterpret_cast<uintptr_t>(rawBuffer.data()), length);
    memoryView.call<void>("set", typedArray);
    return rawBuffer;
}

std::string PublicPppEngine::setImage(const val & typedArray) const
{
    LOG(INFO) << "Setting image ...";
    RawBuffer b = toRawBuffer(typedArray);
    return setImageFromBuffer(b.data(), b.size());
}
#endif
} // namespace ppp

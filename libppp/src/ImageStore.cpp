
#include <iomanip>
#include <opencv2/imgcodecs.hpp>
#include <regex>

#include "EasyExif.h"
#include "ImageStore.h"
#include "Utilities.h"

namespace ppp
{
std::string ImageStore::storeImageData(const cv::Mat & image, const easyexif::EXIFInfoSPtr & exifInfo)
{
    const auto crc32val = Utilities::crc32(0, image.datastart, image.dataend);
    std::stringstream s;
    s << std::setfill('0') << std::setw(8) << std::hex << crc32val;
    const auto && imageKey = s.str();

    {
        std::lock_guard<std::mutex> lg(m_mutex);
        const auto it = m_imageKeyOrder.insert(m_imageKeyOrder.end(), imageKey);
        m_imageCollection[imageKey] = ImageData { image, exifInfo, it };
    }

    handleStoreSize();

    return imageKey;
}

easyexif::EXIFInfoSPtr ImageStore::decodeExifInfo(const BYTE * bufferData, const size_t bufferLength)
{
    easyexif::EXIFInfoSPtr exifInfo = std::make_shared<easyexif::EXIFInfo>();
    if (exifInfo->parseFrom((bufferData), bufferLength) != PARSE_EXIF_SUCCESS)
    {
        return nullptr;
    }
    return exifInfo;
}

std::string ImageStore::setImage(const std::string & imageFilePath)
{
    std::ifstream file(imageFilePath, std::ios::binary);
    std::vector<char> imageFileData { std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>() };
    return setImage(imageFileData.data(), imageFileData.size());
}

std::string ImageStore::setImage(const char * bufferData, const size_t bufferLength)
{
    cv::Mat inputImage;
    easyexif::EXIFInfoSPtr exifInfo;

    if (bufferLength <= 0)
    {
        // Find out if this is a data url
        auto offset = 0;
        auto dataLen = strlen(bufferData);

        static const std::regex re(R"(^data:([a-z]+\/[a-z]+(;[a-z\-]+\=[a-z\-]+)?)?(;base64)?,)");
        std::cmatch cm; // same as std::match_results<const char*> cm;
        if (std::regex_search(bufferData, cm, re))
        {
            offset = cm[0].length();
            dataLen -= offset;
        }

        auto decodedBytes = Utilities::base64Decode(bufferData + offset, dataLen);
        const auto decodedBytesSize = static_cast<int>(decodedBytes.size());
        const cv::_InputArray inputArray(decodedBytes.data(), decodedBytesSize);
        inputImage = imdecode(inputArray, cv::IMREAD_COLOR);
        exifInfo = decodeExifInfo(decodedBytes.data(), decodedBytesSize);
    }
    else
    {
        const cv::_InputArray inputArray(bufferData, static_cast<int>(bufferLength));
        inputImage = imdecode(inputArray, cv::IMREAD_COLOR);
        exifInfo = decodeExifInfo(reinterpret_cast<const unsigned char *>(bufferData), bufferLength);
    }
    return storeImageData(inputImage, exifInfo);
}

bool ImageStore::containsImage(const std::string & imageKey)
{
    std::lock_guard<std::mutex> lg(m_mutex);
    boostImageToTopCache(imageKey);
    return m_imageCollection.find(imageKey) != m_imageCollection.end();
}

cv::Mat ImageStore::getImage(const std::string & imageKey)
{
    std::lock_guard<std::mutex> lg(m_mutex);
    boostImageToTopCache(imageKey);
    return m_imageCollection[imageKey].image;
}

easyexif::EXIFInfoSPtr ImageStore::getExifInfo(const std::string & imageKey)
{
    std::lock_guard<std::mutex> lg(m_mutex);
    boostImageToTopCache(imageKey);
    return m_imageCollection[imageKey].exifInfo;
}

void ImageStore::setStoreSize(const size_t storeSize)
{
    if (storeSize < 1)
    {
        throw std::runtime_error("Invalid store size, should be greater than zero");
    }
    m_storeSize = storeSize;
    handleStoreSize();
}

void ImageStore::handleStoreSize()
{
    std::lock_guard<std::mutex> lg(m_mutex);
    while (m_imageKeyOrder.size() > m_storeSize)
    {
        const auto & imageKey = m_imageKeyOrder.front();
        m_imageCollection.erase(imageKey);
        m_imageKeyOrder.pop_front();
    }
}

void ImageStore::boostImageToTopCache(const std::string & imageKey)
{
    // Move image to top of the cached list
    auto it = m_imageCollection.find(imageKey);
    if (it != m_imageCollection.end())
    {
        m_imageKeyOrder.erase(it->second.storeListOrder);
        const auto newOrderIt = m_imageKeyOrder.insert(m_imageKeyOrder.end(), imageKey);
        it->second.storeListOrder = newOrderIt;
    }
}
} // namespace ppp

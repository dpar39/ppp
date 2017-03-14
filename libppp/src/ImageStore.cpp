
#include <iomanip>

#include "ImageStore.h"
#include "CommonHelpers.h"


std::string ImageStore::setImage(const cv::Mat &inputImage)
{
    auto crc32val = CommonHelpers::crc32(0, inputImage.datastart, inputImage.dataend);
    std::stringstream s;
    s << std::setfill('0') << std::setw(8) << std::hex << crc32val;
    const auto && imageKey = s.str();

    {
        std::lock_guard<std::mutex> lg(m_mutex);
        m_imageCollection[imageKey] = inputImage;
        m_imageKeynOrder.push(imageKey);
    }

    handleStoreSize();

    return imageKey;;
}

bool ImageStore::containsImage(const std::string &imageKey) const
{
    std::lock_guard<std::mutex> lg(m_mutex);
    return m_imageCollection.find(imageKey) != m_imageCollection.end();
}

cv::Mat ImageStore::getImage(const std::string &imageKey)
{
    std::lock_guard<std::mutex> lg(m_mutex);
    return m_imageCollection[imageKey];
}

void ImageStore::setStoreSize(size_t storeSize)
{
    if (storeSize < 1)
    {
        throw std::logic_error("Invalid store size, should be greater than zero");
    }
    m_storeSize = storeSize;
    handleStoreSize();
}

void ImageStore::handleStoreSize()
{
    std::lock_guard<std::mutex> lg(m_mutex);
    while(m_imageKeynOrder.size() > m_storeSize)
    {
        const auto &imageKey = m_imageKeynOrder.front();
        m_imageCollection.erase(imageKey);
        m_imageKeynOrder.pop();
    }
}


#include <iomanip>

#include "ImageStore.h"
#include "Utilities.h"


std::string ImageStore::setImage(const cv::Mat &inputImage)
{
    auto crc32val = Utilities::crc32(0, inputImage.datastart, inputImage.dataend);
    std::stringstream s;
    s << std::setfill('0') << std::setw(8) << std::hex << crc32val;
    const auto && imageKey = s.str();

    {
        std::lock_guard<std::mutex> lg(m_mutex);
        auto it = m_imageKeyOrder.insert(m_imageKeyOrder.end(), imageKey);
        m_imageCollection[imageKey] = ImageOrderPair(inputImage, it);
    }

    handleStoreSize();

    return imageKey;;
}

bool ImageStore::containsImage(const std::string &imageKey) 
{
    std::lock_guard<std::mutex> lg(m_mutex);
    boostImageToTopCache(imageKey);
    return m_imageCollection.find(imageKey) != m_imageCollection.end();
}

cv::Mat ImageStore::getImage(const std::string &imageKey)
{
    std::lock_guard<std::mutex> lg(m_mutex);
    boostImageToTopCache(imageKey);
    return m_imageCollection[imageKey].first;
}

void ImageStore::setStoreSize(size_t storeSize)
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
    while(m_imageKeyOrder.size() > m_storeSize)
    {
        const auto &imageKey = m_imageKeyOrder.front();
        m_imageCollection.erase(imageKey);
        m_imageKeyOrder.pop_front();
    }
}

void ImageStore::boostImageToTopCache(const std::string &imageKey)
{
    // Move image to top of the cached list
    auto it = m_imageCollection.find(imageKey);
    if (it != m_imageCollection.end())
    {
        m_imageKeyOrder.erase(it->second.second);
        auto newOrderIt = m_imageKeyOrder.insert(m_imageKeyOrder.end(), imageKey);
        it->second.second = newOrderIt;
    }
}

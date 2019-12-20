#pragma once

#include "IImageStore.h"

#include <list>
#include <mutex>
#include <unordered_map>

#include <opencv2/core/core.hpp>

namespace ppp
{

FWD_DECL(ImageStore)
struct ImageData final
{
    cv::Mat image;
    easyexif::EXIFInfoSPtr exifInfo;
    LandMarksSPtr landMarks;
    std::list<std::string>::iterator storeListOrder; ///<- Where in the image store order it is located
};

class ImageStore final : public IImageStore
{
public:
    std::string setImage(const std::string & imageFilePath) override;

    std::string setImage(const char * bufferData, size_t bufferLength) override;

    bool containsImage(const std::string & imageKey) override;

    void setStoreSize(size_t storeSize) override;

    cv::Mat getImage(const std::string & imageKey) override;

    LandMarksSPtr getLandMarks(const std::string & imageKey) override;

    easyexif::EXIFInfoSPtr getExifInfo(const std::string & imageKey) override;

protected:
    void configureInternal(const ConfigLoaderSPtr & config) override;

private:
    ///<- Stores the images currently being processing
    std::unordered_map<std::string, ImageData> m_imageCollection;

    ///<- Store the image keys in the order they were added to the store
    std::list<std::string> m_imageKeyOrder;

    ///<- When the number of images in the store is bigger store size,
    ///<- oldest images are to be deleted
    size_t m_storeSize = 1;

    mutable std::mutex m_mutex;

private:
    ///<- Keeps the amount of images in the store to a maximum specified by m_storeSize
    void handleStoreSize();

    void boostImageToTopCache(const std::string & imageKey);

    std::string storeImageData(const cv::Mat & image, const easyexif::EXIFInfoSPtr & exifInfo = nullptr);

    static easyexif::EXIFInfoSPtr decodeExifInfo(const BYTE * bufferData, const size_t bufferLength);
};
} // namespace ppp

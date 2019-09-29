#pragma once

#include "CommonHelpers.h"
#include <opencv2/core/core.hpp>

FWD_DECL(IImageStore)
namespace easyexif
{
FWD_DECL(EXIFInfo);
}

/*!@brief Caches input images that are going to be processed.
 * Only a certain amount of images are kept at any point in time. */
class IImageStore : noncopyable
{
public:
    /*!@brief Loads an image from file and returns the imageKey for later retrieval !*/
    virtual std::string setImage(const std::string & imageFilePath) = 0;

    /*!@brief Decodes and store the image from bytes and computes an image key for latter retrieval !*/
    virtual std::string setImage(const char * bufferData, size_t bufferLength) = 0;

    /*!@brief Gets a copy the image from the store !*/
    virtual cv::Mat getImage(const std::string & imageKey) = 0;

    /*!@brief Gets the image EXIF info if available !*/
    virtual easyexif::EXIFInfoSPtr getExifInfo(const std::string & imageKey) = 0;

    /*!@brief Returns whether an image with the specified key is in the store !*/
    virtual bool containsImage(const std::string & imageKey) = 0;

    /*!@brief Sets the maximum number of images to be stored.
     * If the number set is less than the current store size,
     * the oldest images are removed from the store !*/
    virtual void setStoreSize(size_t storeSize) = 0;

    virtual ~IImageStore() = default;
};

#pragma once

#include <string>
#include <opencv2/core/core.hpp>
#include "CommonHelpers.h"

/*!@brief Caches input images that are going to be processed.
 * Only a certain amount of images are kept at any point in time. */
class IImageStore : noncopyable
{
public:
    /*!@brief Stores the images and computes an image key for latter retrieval !*/
    virtual std::string setImage(const cv::Mat &inputImage) = 0;

    /*!@brief Gets a copy the image from the store !*/
    virtual cv::Mat getImage(const std::string &imageKey) = 0;

    /*!@brief Returns wheter an image with the specified key is in the store !*/
    virtual bool containsImage(const std::string &imageKey) const = 0;

    /*!@brief Sets the maximum number of images to be stored. 
     * If the number set is less than the current store size,
     * the oldest images are removed from the store !*/
    virtual void setStoreSize(size_t storeSize) = 0;

    virtual ~IImageStore() = default;
};
#pragma once

#include "IImageStore.h"

#include <unordered_map>
#include <list>
#include <mutex>

#include <utility>

typedef std::pair<cv::Mat, std::list<std::string>::iterator> ImageOrderPair;

FWD_DECL(ImageStore)

class ImageStore : public IImageStore
{
public:

    std::string setImage(const cv::Mat &image) override;

    bool containsImage(const std::string &imageKey) override;
    

    cv::Mat getImage(const std::string &imageKey) override;

    void setStoreSize(size_t storeSize) override;
private:
    ///<- Stores the images currently being processing
    std::unordered_map<std::string, ImageOrderPair> m_imageCollection;

    ///<- Store the image keys in the order they were added to the store
    std::list<std::string> m_imageKeyOrder;

    ///<- When the number of images in the store is bigger store size,
    ///<- oldest images are to be deleted
    size_t m_storeSize = 1;

    mutable std::mutex m_mutex;

private:
    ///<- Keeps the amount of images in the store to a maximum specified by m_storeSize
    void handleStoreSize();

    void boostImageToTopCache(const std::string& imageKey);
};

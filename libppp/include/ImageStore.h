#pragma once

#include "IImageStore.h"

#include <unordered_map>
#include <queue>
#include <mutex>


class ImageStore : public IImageStore
{
public:

    std::string setImage(const cv::Mat &image) override;

    bool containsImage(const std::string &imageKey) const override;

    cv::Mat getImage(const std::string &imageKey) override;

    void setStoreSize(size_t storeSize) override;
private:
    ///<- Stores the images currently being processing
    std::unordered_map<std::string, cv::Mat> m_imageCollection;

    ///<- Store the image keys in the order they were added to the store
    std::queue<std::string> m_imageKeynOrder;

    ///<- When the number of images in the store is bigger store size,
    ///<- oldest images are to be deleted
    size_t m_storeSize = 1;

    mutable std::mutex m_mutex;

private:
    ///<- Keeps the amount of images in the store to a maximum specified by m_storeSize
    void handleStoreSize();
};

#pragma once

#include "IImageStore.h"

#include <unordered_map>


class ImageStore : public IImageStore
{
public:

    std::string setImage(const cv::Mat &image) override;

    bool containsImage(const std::string &imageKey) override;

    const cv::Mat &getImage(const std::string &imageKey) override;
private:
    std::unordered_map<std::string, cv::Mat> m_imageCollection;
   
};
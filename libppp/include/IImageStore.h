#pragma once

#include <string>
#include <opencv2/core/core.hpp>
#include "CommonHelpers.h"


class IImageStore : noncopyable
{
public:
    virtual std::string setImage(const cv::Mat &inputImage) = 0;

    virtual const cv::Mat &getImage(const std::string &imageKey) = 0;

    virtual bool containsImage(const std::string &imageKey) = 0;

    virtual ~IImageStore() = default;
};
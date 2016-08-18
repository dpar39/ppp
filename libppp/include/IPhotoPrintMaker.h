#pragma once

#include <opencv2/core/core.hpp>
class IPhotoPrintMaker
{
public:
    virtual cv::Mat cropPicture(const cv::Mat& originalImage,
                                const cv::Point& crownPoint,
                                const cv::Point& chinPoint,
                                const PhotoStandard& ps) = 0;
};
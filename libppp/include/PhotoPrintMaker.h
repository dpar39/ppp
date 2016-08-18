#pragma once

#include "IPhotoPrintMaker.h"

class PhotoPrintMaker : public IPhotoPrintMaker
{
public:
    virtual cv::Mat cropPicture(const cv::Mat& originalImage,
                                const cv::Point& crownPoint,
                                const cv::Point& chinPoint,
                                const PhotoStandard& ps);
};
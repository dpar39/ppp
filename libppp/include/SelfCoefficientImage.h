#pragma once

#include "ISelfCoefficientCalculator.h"


class SelfCoefficientImage : public ISelfCoefficientImage
{
public:
    virtual ~SelfCoefficientImage() = default;

    void compute(const cv::Mat& inputImg, cv::Mat &outputImg, int maskSize) override;
};
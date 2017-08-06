#pragma once

#include "ISelfCoefficientCalculator.h"


class SelfCoefficientCalculator : public ISelfCoefficientCalculator
{
public:
    virtual ~SelfCoefficientCalculator() = default;

    void compute(const cv::Mat& inputImg, cv::Mat outputImg) override;
};
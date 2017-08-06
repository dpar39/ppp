#pragma once

#include <opencv2/core/core.hpp>
#include "CommonHelpers.h"

FWD_DECL(ISelfCoeffientCalculator)

/**
 * \brief Calculates the self-coefficient of  an image
 */
class ISelfCoefficientCalculator : noncopyable
{
public:
    virtual ~ISelfCoefficientCalculator() = default;

    virtual void compute(const cv::Mat &inputImg, cv::Mat outputImg) = 0;
};

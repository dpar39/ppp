#pragma once

#include <opencv2/core/core.hpp>
#include <rapidjson/document.h>
#include "CommonHelpers.h"

namespace cv
{
    class CascadeClassifier;
}
struct LandMarks;


class IDetector : noncopyable
{
public:
    /*!@brief Configures the detector from Json data !*/
    virtual void configure(rapidjson::Value &config) = 0;

    /*!@brief Detects a subset landmarks and stores them !
    *  @returns true if the intended landmarks were detected and can be used as input for subsequent detection, false otherwise !*/
    virtual bool detectLandMarks(const cv::Mat& inputImage, LandMarks &landmarks) = 0;

    virtual ~IDetector() = default;
};
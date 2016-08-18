#pragma once

#include <memory>
#include <opencv2/core/core.hpp>
#include <rapidjson/document.h>

namespace cv
{
    class CascadeClassifier;
}
struct LandMarks;


class IDetector
{
public:
    virtual void configure(rapidjson::Value &config) = 0;

    virtual bool detectLandMarks(const cv::Mat& inputImage, LandMarks &landmarks) = 0;

    virtual ~IDetector() { }
};
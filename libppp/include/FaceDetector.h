#pragma once

#include <memory>
#include "IDetector.h"

namespace cv
{
    class CascadeClassifier;
}

struct LandMarks;

DECLARE(FaceDetector)

class FaceDetector : public IDetector
{
public:
    void configure(rapidjson::Value &cfg) override;

    bool detectLandMarks(const cv::Mat& inputImage, LandMarks &landmarks) override;

private:
    std::shared_ptr<cv::CascadeClassifier> m_pFaceCascadeClassifier;

    void calculateScaleSearch(const cv::Size& inputImageSize, double minFaceRatio, double maxFaceRatio, cv::Size& minFaceSize, cv::Size &maxFaceSize) const;

};
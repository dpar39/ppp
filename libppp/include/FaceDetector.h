#pragma once

#include <memory>
#include "IDetector.h"

struct LandMarks;

FWD_DECL(FaceDetector)

class FaceDetector : public IDetector
{
public:
    void configure(rapidjson::Value &cfg) override;

    bool detectLandMarks(const cv::Mat& inputImage, LandMarks &landmarks) override;

private:
    cv::CascadeClassifierSPtr m_pFaceCascadeClassifier;

    void calculateScaleSearch(const cv::Size& inputImageSize, double minFaceRatio, double maxFaceRatio, cv::Size& minFaceSize, cv::Size &maxFaceSize) const;

};
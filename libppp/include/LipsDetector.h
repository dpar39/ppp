#pragma once

#include <memory>
#include "IDetector.h"

class LipsDetector : public IDetector
{
public:

    virtual void configure(rapidjson::Value &config) override;

    virtual bool detectLandMarks(const cv::Mat& inputImage, LandMarks &landmarks) override;

private:
    std::shared_ptr<cv::CascadeClassifier> m_pMouthDetector;
    bool m_useHaarCascades;
};
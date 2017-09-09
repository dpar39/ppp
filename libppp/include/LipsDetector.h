#pragma once

#include <memory>
#include "IDetector.h"

class LipsDetector : public IDetector
{
public:

    void configure(rapidjson::Value &config) override;

    bool detectLandMarks(const cv::Mat& inputImage, LandMarks &landmarks) override;

private:

    bool getBeardMask(cv::Mat &mouthAreaImage) const;

    cv::CascadeClassifierSPtr m_pMouthCascadeClassifier;

    bool m_useHaarCascades = true;
    bool m_useColorSegmentationAlgorithm;
};
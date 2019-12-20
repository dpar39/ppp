#pragma once

#include "IDetector.h"

namespace ppp
{

class LipsDetector final : public IDetector
{
    void configureInternal(const ConfigLoaderSPtr & config) override;

public:
    bool detectLandMarks(const cv::Mat & inputImage, LandMarks & landmarks) override;

private:
    bool getBeardMask(cv::Mat & mouthAreaImage) const;

    cv::CascadeClassifierSPtr m_pMouthCascadeClassifier;

    bool m_useHaarCascades { true };
    bool m_useColorSegmentationAlgorithm { false };
};
} // namespace ppp

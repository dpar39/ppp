#pragma once

#include "IDetector.h"
#include <dlib/image_processing/frontal_face_detector.h>

namespace ppp
{
FWD_DECL(LandMarks)
FWD_DECL(FaceDetector)

class FaceDetector final : public IDetector
{
    void configureInternal(const ConfigLoaderSPtr & config) override;

public:
    bool detectLandMarks(const cv::Mat & inputImage, LandMarks & landmarks) override;

private:
    cv::CascadeClassifierSPtr m_pFaceCascadeClassifier;

    bool m_useDlibFaceDetection { false };

    void calculateScaleSearch(const cv::Size & inputImageSize,
                              double minFaceRatio,
                              double maxFaceRatio,
                              cv::Size & minFaceSize,
                              cv::Size & maxFaceSize) const;

private:
    std::shared_ptr<dlib::frontal_face_detector> m_frontalFaceDetector;
};
} // namespace ppp

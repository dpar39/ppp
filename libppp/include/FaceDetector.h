#pragma once

#include "IDetector.h"
#include <dlib/image_processing/frontal_face_detector.h>

struct LandMarks;

FWD_DECL(FaceDetector)
namespace cv
{
FWD_DECL(CascadeClassifier)
}

class FaceDetector final : public IDetector
{
public:
    void configure(rapidjson::Value & config) override;

    bool detectLandMarks(const cv::Mat & inputImage, LandMarks & landmarks) override;

private:
    cv::CascadeClassifierSPtr m_pFaceCascadeClassifier;

    bool m_useDlibFaceDetection { false };

    void calculateScaleSearch(const cv::Size & inputImageSize,
                              double minFaceRatio,
                              double maxFaceRatio,
                              cv::Size & minFaceSize,
                              cv::Size & maxFaceSize) const;

    std::shared_ptr<dlib::frontal_face_detector> m_frontalFaceDetector;
};

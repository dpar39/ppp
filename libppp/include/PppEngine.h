#pragma once

#include <memory>
#include <rapidjson/document.h>
#include <opencv2/core/core.hpp>

#include "CommonHelpers.h"

#include <dlib/image_processing/frontal_face_detector.h>


struct LandMarks;
FWD_DECL(IDetector)
FWD_DECL(ICrownChinEstimator)
FWD_DECL(IImageStore)
FWD_DECL(IPhotoPrintMaker)

class CanvasDefinition;
class PhotoStandard;

namespace dlib
{
    class shape_predictor;
}

FWD_DECL(PppEngine)

class PppEngine : noncopyable
{
public:
    explicit PppEngine(IDetectorSPtr pFaceDetector = nullptr,
        IDetectorSPtr pEyeDetector = nullptr,
        IDetectorSPtr pLipsDetector = nullptr,
        ICrownChinEstimatorSPtr pCrownChinEstimator = nullptr,
        IPhotoPrintMakerSPtr pPhotoPrintMaker = nullptr,
        IImageStoreSPtr pImageStore = nullptr);

    // Native interface
    bool configure(rapidjson::Value& config);

    std::string setInputImage(const cv::Mat& inputImage) const;

    bool detectLandMarks(const std::string& imageKey, LandMarks& landMarks) const;

    cv::Mat createTiledPrint(const std::string& imageKey, PhotoStandard &ps, CanvasDefinition &canvas, cv::Point &crownMark, cv::Point &chinMark) const;

private:

    IDetectorSPtr m_pFaceDetector;
    IDetectorSPtr m_pEyesDetector;
    IDetectorSPtr m_pLipsDetector;
    ICrownChinEstimatorSPtr m_pCrownChinEstimator;

    IPhotoPrintMakerSPtr m_pPhotoPrintMaker;
    IImageStoreSPtr m_pImageStore;

    std::shared_ptr<dlib::shape_predictor> m_shapePredictor;
    bool m_useDlibLandmarkDetection;

    void verifyImageExists(const std::string& imageKey) const;
};
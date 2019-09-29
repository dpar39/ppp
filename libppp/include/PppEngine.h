#pragma once

#include <memory>
#include <opencv2/core/core.hpp>
#include <rapidjson/document.h>

#include "CommonHelpers.h"

#include <dlib/image_processing/frontal_face_detector.h>
#include <unordered_map>

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

enum class LandMarkType
{
    EYE_LEFT_PUPIL,
    EYE_RIGHT_PUPIL,
    LIPS_LEFT_CORNER,
    LIPS_RIGHT_CORNER,
    CHIN_LOWEST_POINT
};

struct EnumClassHash
{
    template <typename T>
    std::size_t operator()(T t) const
    {
        return static_cast<std::size_t>(t);
    }
};

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
    bool configure(const std::string & configString);

    bool detectLandMarks(const std::string & imageKey, LandMarks & landMarks) const;
    cv::Point getLandMark(const dlib::full_object_detection & shape, LandMarkType type) const;
    cv::Mat cropPicture(const std::string & imageKey,
                        PhotoStandard & ps,
                        CanvasDefinition & canvas,
                        cv::Point & crownMark,
                        cv::Point & chinMark) const;

    cv::Mat createTiledPrint(const std::string & imageKey,
                             PhotoStandard & ps,
                             CanvasDefinition & canvas,
                             cv::Point & crownMark,
                             cv::Point & chinMark) const;
    IImageStoreSPtr getImageStore() const;

private:
    IDetectorSPtr m_pFaceDetector;
    IDetectorSPtr m_pEyesDetector;
    IDetectorSPtr m_pLipsDetector;
    ICrownChinEstimatorSPtr m_pCrownChinEstimator;

    IPhotoPrintMakerSPtr m_pPhotoPrintMaker;
    IImageStoreSPtr m_pImageStore;

    std::shared_ptr<dlib::shape_predictor> m_shapePredictor;
    bool m_useDlibLandmarkDetection;

    std::unordered_map<LandMarkType, std::vector<int>, EnumClassHash> m_landmarkIndexMapping;

    void verifyImageExists(const std::string & imageKey) const;
};

#pragma once

#include "CommonHelpers.h"
#include <opencv2/core/core.hpp>
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
class full_object_detection;
class shape_predictor;
} // namespace dlib

FWD_DECL(PppEngine)

enum class LandMarkType
{
    EYE_PUPIL_CENTER_LEFT,
    EYE_PUPIL_CENTER_RIGHT,
    MOUTH_CORNER_LEFT,
    MOUTH_CORNER_RIGHT,
    CHIN_LOWEST_POINT,
    NOSE_TIP_POINT,
    EYE_OUTER_CORNER_LEFT,
    EYE_OUTER_CORNER_RIGHT

};

struct EnumClassHash
{
    template <typename T>
    std::size_t operator()(T t) const
    {
        return static_cast<std::size_t>(t);
    }
};

class PppEngine final : NonCopyable
{
public:
    explicit PppEngine(const IDetectorSPtr & pFaceDetector = nullptr,
                       const IDetectorSPtr & pEyesDetector = nullptr,
                       const IDetectorSPtr & pLipsDetector = nullptr,
                       const ICrownChinEstimatorSPtr & pCrownChinEstimator = nullptr,
                       const IPhotoPrintMakerSPtr & pPhotoPrintMaker = nullptr,
                       const IImageStoreSPtr & pImageStore = nullptr);

    // Native interface
    bool configure(const std::string & configString);

    bool detectLandMarks(const std::string & imageKey, LandMarks & landMarks) const;

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

    cv::Point getLandMark(const std::vector<cv::Point> & landmarks, LandMarkType type) const;
};

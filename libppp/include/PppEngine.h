#pragma once

#include <memory>
#include <rapidjson/document.h>
#include <opencv2/core/core.hpp>
#include <unordered_map>

#include "CommonHelpers.h"


struct LandMarks;
class IDetector;
class IImageStore;
class IPhotoPrintMaker;
class CanvasDefinition;
class PhotoStandard;

FWD_DECL(PppEngine)

class PppEngine : noncopyable
{
public:
    explicit PppEngine(std::shared_ptr<IDetector> pFaceDetector = nullptr,
        std::shared_ptr<IDetector> pEyeDetector = nullptr,
        std::shared_ptr<IDetector> pLipsDetector = nullptr,
        std::shared_ptr<IPhotoPrintMaker> pPhotoPrintMaker = nullptr,
        std::shared_ptr<IImageStore> pImageStore = nullptr);

    // Native interface
    void configure(rapidjson::Value &config);

    std::string setInputImage(cv::Mat& inputImage);

    bool detectLandMarks(const std::string& imageKey, LandMarks& landMarks);

    cv::Mat createTiledPrint(const std::string& imageKey, PhotoStandard &ps, CanvasDefinition &canvas, cv::Point &crownMark, cv::Point &chinMark);

private:
    /*!@brief Estimate chin and crown point from the available landmarks!
    *  The result is written in the same LandMark structure !*/
    void estimateHeadTopAndChinCorner(LandMarks &landMarks) const;

    std::shared_ptr<IDetector> m_pFaceDetector;
    std::shared_ptr<IDetector> m_pEyesDetector;
    std::shared_ptr<IDetector> m_pLipsDetector;

    std::shared_ptr<IPhotoPrintMaker> m_pPhotoPrintMaker;
    std::shared_ptr<IImageStore> m_pImageStore;

    void verifyImageExists(const std::string& imageKey) const;
};
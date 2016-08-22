#pragma once

#include <memory>
#include <rapidjson/document.h>
#include <opencv2/core/core.hpp>
#include <unordered_map>
#include <functional>
#include <vector>

struct LandMarks;
class IDetector;
class IPhotoPrintMaker;
class CanvasDefinition;
class PhotoStandard;

class PppEngine
{
public:
    explicit PppEngine(std::shared_ptr<IDetector> pFaceDetector = nullptr,
        std::shared_ptr<IDetector> pEyeDetector = nullptr,
        std::shared_ptr<IDetector> pLipsDetector = nullptr,
        std::shared_ptr<IPhotoPrintMaker> pPhotoPrintMaker = nullptr);

    // Native interface
    void configure(rapidjson::Value &config);

    std::string setInputImage(cv::Mat& inputImage);

    bool detectLandMarks(const std::string& imageKey, LandMarks& landMarks);

    cv::Mat createTiledPrint(const std::string& imageKey, PhotoStandard &ps, CanvasDefinition &canvas, cv::Point &crownMark, cv::Point &chinMark);

private:
    /*!@brief Estimate chin and crown point from the available landmarks!
    *  The result is written in the same LandMark structure !*/
    void estimateHeadTopAndChinCorner(LandMarks &landMarks);

    std::shared_ptr<IDetector> m_pFaceDetector;
    std::shared_ptr<IDetector> m_pEyesDetector;
    std::shared_ptr<IDetector> m_pLipsDetector;

    std::shared_ptr<IPhotoPrintMaker> m_pPhotoPrintMaker;

    std::unordered_map<std::string, cv::Mat> m_imageCollection;

    void verifyImageExists(const std::string& imageKey);

public:
    std::shared_ptr<PppEngine> fromJson(const std::string &config);
};
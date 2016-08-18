#pragma once

#include <memory>
#include <rapidjson/document.h>
#include <opencv2/core/mat.hpp>

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

    void configure(rapidjson::Value &config);

public: // Native interface

    void setInputImage(cv::Mat &inputImage)
    {
        m_inputImage = inputImage;
    }

    bool detectLandMarks(LandMarks &faceLandMarks);

    cv::Mat createTiledPrint(PhotoStandard &ps, CanvasDefinition &canvas, cv::Point &crownMark, cv::Point &chinMark);

private:
    /*!@brief Estimate chin and crown point from the available landmarks!
    *  The result is written in the same LandMark structure !*/
    void estimateHeadTopAndChinCorner(LandMarks &landMarks);

    std::shared_ptr<IDetector> m_pFaceDetector;
    std::shared_ptr<IDetector> m_pEyesDetector;
    std::shared_ptr<IDetector> m_pLipsDetector;

    std::shared_ptr<IPhotoPrintMaker> m_pPhotoPrintMaker;

    cv::Mat m_inputImage;

public:
    std::shared_ptr<PppEngine> fromJson(const std::string &config);
};
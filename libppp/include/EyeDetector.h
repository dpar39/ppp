#pragma once
#include "IDetector.h"
#include "CommonHelpers.h"

#include <memory>
#include <type_traits>

namespace cv
{
    class CascadeClassifier;
}

FWD_DECL(EyeDetector)

class EyeDetector : public IDetector
{
public:

    void configure(rapidjson::Value &cfg) override;

    bool detectLandMarks(const cv::Mat& inputImage, LandMarks &landmarks) override;

private:
    cv::Mat m_leftCornerKernel;
    cv::Mat m_rightCornerKernel;
    cv::Mat m_xGradKernel;
    cv::Mat m_yGradKernel;

private:  // Configuration

    bool m_useHaarCascades = false;
    std::shared_ptr<cv::CascadeClassifier> m_leftEyeCascadeClassifier;
    std::shared_ptr<cv::CascadeClassifier> m_rightEyeCascadeClassifier;

    // Definition of the search areas to locate pupils expressed as the ratios of the face rectangle
    const double m_topFaceRatio = 0.28;  ///<- Distance from the top of the face 
    const double m_sideFaceRatio = 0.13; ///<- Distance from the sides of the face rectangle
    const double m_widthRatio = 0.35;    ///<- ROI width ratio with respect to head size
    const double m_heightRatio = 0.25;   ///<- ROI height ratio with respect to head size

    // Preprocessing
    const bool kSmoothFaceImage = false;
    const float kSmoothFaceFactor = 0.005f;

    // Algorithm Parameters
    const int kFastEyeWidth = 50;
    const int kWeightBlurSize = 5;
    const bool kEnableWeight = false;
    const float kWeightDivisor = 150.0;
    const double kGradientThreshold = 50.0;

    // Post-processing
    const bool kEnablePostProcess = true;
    const float kPostProcessThreshold = 0.97f;

    // Eye Corner detection
    const bool kEnableEyeCorner = false;
    

private:
    static void validateAndApplyFallbackIfRequired(const cv::Size &eyeRoiSize, cv::Point &eyeCenter);

    static cv::Rect detectWithHaarCascadeClassifier(const cv::Mat &img, cv::CascadeClassifier *cc);

    cv::Point findEyeCenter(const cv::Mat& image);

    void createCornerKernels();

    cv::Mat eyeCornerMap(const cv::Mat& region, bool left, bool left2) const;

    void testPossibleCentersFormula(int x, int y, unsigned char weight, double gx, double gy, cv::Mat& out);

    cv::Mat floodKillEdges(cv::Mat& mat);

    cv::Mat matrixMagnitude(const cv::Mat& matX, const cv::Mat& matY);

    double computeDynamicThreshold(const cv::Mat& mat, double stdDevFactor) const;

    cv::Point2f findEyeCorner(cv::Mat region, bool left, bool left2) const;

    static cv::Point2f findSubpixelEyeCorner(cv::Mat region, cv::Point maxP);

    cv::Point unscalePoint(cv::Point p, cv::Rect origSize) const;

    void scaleToFastSize(const cv::Mat& src, cv::Mat& dst) const;
};

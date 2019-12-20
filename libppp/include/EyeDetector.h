#pragma once
#include "CommonHelpers.h"
#include "IDetector.h"

namespace ppp
{
FWD_DECL(EyeDetector)

class EyeDetector final : public IDetector
{

public:
    bool detectLandMarks(const cv::Mat & grayImage, LandMarks & landMarks) override;

protected:
    void configureInternal(const ConfigLoaderSPtr & cfg) override;

private:
    cv::Mat m_leftCornerKernel;
    cv::Mat m_rightCornerKernel;
    cv::Mat m_xGradKernel;
    cv::Mat m_yGradKernel;

private: // Configuration
    bool m_useHaarCascades = false;
    cv::CascadeClassifierSPtr m_leftEyeCascadeClassifier;
    cv::CascadeClassifierSPtr m_rightEyeCascadeClassifier;

    // Definition of the search areas to locate pupils expressed as the ratios of the face rectangle
    static constexpr double m_topFaceRatio = 0.28; ///<- Distance from the top of the face
    static constexpr double m_sideFaceRatio = 0.13; ///<- Distance from the sides of the face rectangle
    static constexpr double m_widthRatio = 0.35; ///<- ROI width ratio with respect to head size
    static constexpr double m_heightRatio = 0.25; ///<- ROI height ratio with respect to head size

    // Preprocessing
    const bool m_smoothFaceImage = false;
    const float m_smoothFaceFactor = 0.005f;

    // Algorithm Parameters
    const int kFastEyeWidth = 50;
    const int kWeightBlurSize = 5;
    const bool kEnableWeight = false;
    const float kWeightDivisor = 150.0;
    const double kGradientThreshold = 50.0;

    // Post-processing
    const bool m_enablePostProcess = true;
    const float m_postProcessThreshold = 0.97f;

private:
    static void validateAndApplyFallbackIfRequired(const cv::Size & eyeRoiSize, cv::Point & eyeCenter);

    static cv::Rect detectWithHaarCascadeClassifier(const cv::Mat & image, cv::CascadeClassifier * cc);

    cv::Point findEyeCenter(const cv::Mat & image) const;

    void createCornerKernels();

    void testPossibleCentersFormula(int x, int y, unsigned char weight, double gx, double gy, cv::Mat & out) const;

    cv::Mat floodKillEdges(cv::Mat & mat) const;

    cv::Mat matrixMagnitude(const cv::Mat & matX, const cv::Mat & matY) const;

    double computeDynamicThreshold(const cv::Mat & mat, double stdDevFactor) const;

    cv::Point unscalePoint(cv::Point p, cv::Rect origSize) const;

    void scaleToFastSize(const cv::Mat & src, cv::Mat & dst) const;
};

} // namespace ppp

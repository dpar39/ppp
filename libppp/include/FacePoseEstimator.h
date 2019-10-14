#pragma once
#include "IFacePoseEstimator.h"

#include <opencv2/core/mat.hpp>
#include <opencv2/core/types.hpp>
#include <vector>

class FacePoseEstimator final : public IFacePoseEstimator
{
public:
    void configure();

    void estimatePose(const LandMarks & landMarks, double focalLength, cv::Point2d focalCenter) const override;

private:
    std::vector<cv::Point3d> m_modelPoints;

    cv::Mat rotationVector; // Rotation in axis-angle form
    cv::Mat translationVector;
};

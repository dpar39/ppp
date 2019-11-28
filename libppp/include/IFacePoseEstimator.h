#pragma once
#include "CommonHelpers.h"
#include <opencv2/core/types.hpp>

namespace ppp
{
FWD_DECL(LandMarks);

class IFacePoseEstimator
{
public:
    virtual ~IFacePoseEstimator() = default;

    virtual cv::Vec3d estimatePose(const LandMarksSPtr & landMarks, double focalLength, cv::Point2d focalCenter) = 0;
};
} // namespace ppp

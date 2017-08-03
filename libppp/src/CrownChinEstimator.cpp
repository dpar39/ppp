#include "CrownChinEstimator.h"
#include "Geometry.h"
#include "LandMarks.h"

void CrownChinEstimator::configure(rapidjson::Value& config)
{
    auto& lipsDetectorCfg = config["crownChinEstimator"];
    m_chinCrownCoeff = lipsDetectorCfg["chinCrownCoeff"].GetDouble();
    m_chinFrownCoeff = lipsDetectorCfg["chinFrownCoeff"].GetDouble();
}

bool CrownChinEstimator::estimateCrownChin(LandMarks& landMarks)
{
    // Using normalised distance to be the sum of the distance between eye pupils and the distance mouth to frown
    // Distance chin to crown is estimated as 1.7699 of that value with correlation 0.7954
    // Distance chin to frown is estimated as 0.8945 of that value with correlation 0.8426

    auto frownPointPix = cv::Point2d(landMarks.eyeLeftPupil + landMarks.eyeRightPupil) *0.5;

    cv::Point2d mouthCenterPoint;
    auto mouthAve = 0;
    if (cv::norm(landMarks.lipLeftCorner - landMarks.lipRightCorner) > 0)
    {
        mouthCenterPoint += cv::Point2d(landMarks.lipLeftCorner + landMarks.lipRightCorner) * 0.5;
        ++mouthAve;
    }
    if (landMarks.vjMouthRect.area() > 0)
    {
        mouthCenterPoint += cv::Point2d(landMarks.vjMouthRect.br() + landMarks.vjMouthRect.tl()) * 0.5;
        ++mouthAve;
    }
    if (mouthAve > 0)
    {
        mouthCenterPoint /= mouthAve;
    }

    auto normalisedDistancePixels = cv::norm(landMarks.eyeLeftPupil - landMarks.eyeRightPupil)
        + cv::norm(frownPointPix - mouthCenterPoint);

    auto chinFrownDistancePix = m_chinFrownCoeff * normalisedDistancePixels;
    auto chinCrownDistancePix = m_chinCrownCoeff * normalisedDistancePixels;

    landMarks.chinPoint = pointInLineAtDistance(frownPointPix, mouthCenterPoint, chinFrownDistancePix);
    landMarks.crownPoint = pointInLineAtDistance(landMarks.chinPoint, frownPointPix, chinCrownDistancePix);

    return true;
}

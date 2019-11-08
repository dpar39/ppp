#include "CrownChinEstimator.h"
#include "LandMarks.h"
#include "Utilities.h"

namespace ppp
{

void CrownChinEstimator::configure(rapidjson::Value & config)
{
    auto & lipsDetectorCfg = config["crownChinEstimator"];
    m_chinCrownCoeff = lipsDetectorCfg["chinCrownCoeff"].GetDouble();
    m_chinFrownCoeff = lipsDetectorCfg["chinFrownCoeff"].GetDouble();
}

bool CrownChinEstimator::estimateCrownChin(LandMarks & landMarks)
{
    // Using normalised distance to be the sum of the distance between eye pupils and the distance mouth to frown
    // Distance chin to crown is estimated as 1.7699 of that value with correlation 0.7954
    // Distance chin to frown is estimated as 0.8945 of that value with correlation 0.8426

    const auto frownPointPix = cv::Point2d(landMarks.eyeLeftPupil + landMarks.eyeRightPupil) * 0.5;

    cv::Point2d mouthCenterPoint;
    auto mouthAve = 0;
    if (norm(landMarks.lipLeftCorner - landMarks.lipRightCorner) > 0)
    {
        mouthCenterPoint += cv::Point2d(landMarks.lipLeftCorner + landMarks.lipRightCorner) * 0.5;
        ++mouthAve;
    }
    if (landMarks.vjMouthRect.area() > 0)
    {
        // mouthCenterPoint += cv::Point2d(landMarks.vjMouthRect.br() + landMarks.vjMouthRect.tl()) * 0.5;
        //++mouthAve;
    }
    if (mouthAve > 0)
    {
        mouthCenterPoint /= mouthAve;
    }

    const auto normalisedDistancePixels = norm(landMarks.eyeLeftPupil - landMarks.eyeRightPupil)
        + norm(frownPointPix - mouthCenterPoint);

    const auto chinFrownDistancePix = m_chinFrownCoeff * normalisedDistancePixels;
    const auto chinCrownDistancePix = m_chinCrownCoeff * normalisedDistancePixels;

    if (landMarks.chinPoint == cv::Point())
    {
        landMarks.chinPoint = Utilities::pointInLineAtDistance(frownPointPix, mouthCenterPoint, chinFrownDistancePix);
    }
    landMarks.crownPoint = Utilities::pointInLineAtDistance(landMarks.chinPoint, frownPointPix, chinCrownDistancePix);
    return true;
}
} // namespace ppp

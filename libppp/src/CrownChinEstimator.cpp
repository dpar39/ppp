#include "CrownChinEstimator.h"
#include "Geometry.h"
#include "LandMarks.h"

void CrownChinEstimator::configure(rapidjson::Value& config)
{
}

bool CrownChinEstimator::estimateCrownChin(LandMarks& landMarks)
{
    // Using normalised distance to be the sum of the distance between eye pupils and the distance mouth to frown
    // Distance chin to crown is estimated as 1.7699 of that value with correlation 0.7954
    // Distance chin to frown is estimated as 0.8945 of that value with correlation 0.8426
    const auto chinCrownCoeff = 1.7699;
    const auto chinFrownCoeff = 0.8945;

    auto frownPointPix = (landMarks.eyeLeftPupil + landMarks.eyeRightPupil) / 2;
    auto mouthPointPix = (landMarks.lipLeftCorner + landMarks.lipRightCorner) / 2;
    auto normalisedDistancePixels = cv::norm(landMarks.eyeLeftPupil - landMarks.eyeRightPupil)
        + cv::norm(frownPointPix - mouthPointPix);

    auto chinFrownDistancePix = chinFrownCoeff * normalisedDistancePixels;
    auto chinCrownDistancePix = chinCrownCoeff * normalisedDistancePixels;

    landMarks.chinPoint = pointInLineAtDistance(frownPointPix, mouthPointPix, chinFrownDistancePix);
    landMarks.crownPoint = pointInLineAtDistance(landMarks.chinPoint, frownPointPix, chinCrownDistancePix);

    return true;
}

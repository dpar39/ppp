#include "CrownChinEstimator.h"
#include "ConfigLoader.h"
#include "LandMarks.h"
#include "Utilities.h"

namespace ppp {

void CrownChinEstimator::configure(const ConfigLoaderSPtr & config)
{
    const auto & coeffConfig = config->get({ "crownChinEstimator" });
    m_chinCrownCoeff = coeffConfig["chinCrownCoeff"].GetDouble();
    m_chinFrownCoeff = coeffConfig["chinFrownCoeff"].GetDouble();
}

bool CrownChinEstimator::estimateCrownChin(LandMarks & landMarks)
{
    // Using normalized distance to be the sum of the distance between eye pupils and the distance mouth to frown
    // Distance chin to crown is estimated as 1.7699 of that value with correlation 0.7954
    // Distance chin to frown is estimated as 0.8945 of that value with correlation 0.8426

    const auto frownPointPix = cv::Point2d(landMarks.eyeLeftPupil + landMarks.eyeRightPupil) * 0.5;

    const auto normalizedDistancePixels = norm(landMarks.eyeLeftPupil - landMarks.eyeRightPupil)
        + norm(frownPointPix - cv::Point2d(landMarks.chinPoint));

    const auto chinFrownDistancePix = m_chinFrownCoeff * normalizedDistancePixels;
    const auto chinCrownDistancePix = m_chinCrownCoeff * normalizedDistancePixels;

    landMarks.crownPoint = Utilities::pointInLineAtDistance(landMarks.chinPoint, frownPointPix, chinCrownDistancePix);
    return true;
}
} // namespace ppp

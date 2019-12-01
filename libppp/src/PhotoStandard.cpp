#include "PhotoStandard.h"
#include "Utilities.h"

namespace ppp
{
PhotoStandard::PhotoStandard(const double picWidth,
                             const double picHeight,
                             const double faceHeight,
                             double crownTop,
                             const double eyeLineBottom,
                             const double picResolution,
                             const std::string & units)
{
    VALIDATE_GT(picWidth, 0);
    VALIDATE_GT(picHeight, 0);
    VALIDATE_GT(faceHeight, 0);
    VALIDATE_GT(picResolution, 0);
    VALIDATE_GE(crownTop, 0);
    VALIDATE_GE(eyeLineBottom, 0);
    VALIDATE_LT(faceHeight, picHeight);

    if (crownTop <= 0 && eyeLineBottom > 0)
    {
        // Convert eye line to bottom distance into crown top distance
        constexpr const auto alpha = 0.477196;
        crownTop = picHeight - eyeLineBottom - alpha * faceHeight;
    }

    VALIDATE_LT(crownTop, picHeight - faceHeight);

    m_picWidth_mm = Utilities::toMM(picWidth, units);
    m_picHeight_mm = Utilities::toMM(picHeight, units);
    m_faceHeight_mm = Utilities::toMM(faceHeight, units);
    m_crownTop_mm = Utilities::toMM(crownTop, units);
    m_picResolution_dpi = picResolution;
}

double PhotoStandard::photoWidthMM() const
{
    return m_picWidth_mm;
}

double PhotoStandard::photoHeightMM() const
{
    return m_picHeight_mm;
}

double PhotoStandard::faceHeightMM() const
{
    return m_faceHeight_mm;
}

double PhotoStandard::crownTopMM() const
{
    return m_crownTop_mm;
}

double PhotoStandard::resolutionDpi() const
{
    return m_picResolution_dpi;
}

std::shared_ptr<PhotoStandard> PhotoStandard::fromJson(const rapidjson::Value & photoStandardJson)
{
    const auto picHeight = photoStandardJson[PHOTO_HEIGHT].GetDouble();
    const auto picWidth = photoStandardJson[PHOTO_WIDTH].GetDouble();
    const auto faceHeight = photoStandardJson[PHOTO_FACE_HEIGHT].GetDouble();
    const auto picResolution = Utilities::getField(photoStandardJson, PHOTO_RESOLUTION, 300.0);
    const auto units = Utilities::getField(photoStandardJson, UNITS, std::string("mm"));
    const auto crownTop = Utilities::getField(photoStandardJson, PHOTO_CROWN_TOP, 0.0);
    const auto bottomEyeLine = Utilities::getField(photoStandardJson, PHOTO_EYELINE_BOTTOM, 0.0);

    return std::make_shared<PhotoStandard>(picWidth,
                                           picHeight,
                                           faceHeight,
                                           crownTop,
                                           bottomEyeLine,
                                           picResolution,
                                           units);
}
} // namespace ppp

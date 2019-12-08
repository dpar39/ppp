#include "PhotoStandard.h"
#include "Utilities.h"

namespace ppp
{
PhotoStandard::PhotoStandard(const double photoWidth,
                             const double photoHeight,
                             const double faceHeight,
                             double crownTop,
                             const double eyeLineBottom,
                             const double picResolution,
                             const std::string & units)
{
    VALIDATE_GT(photoWidth, 0);
    VALIDATE_GT(photoHeight, 0);
    VALIDATE_GT(faceHeight, 0);
    VALIDATE_GT(picResolution, 0);
    VALIDATE_GE(crownTop, 0);
    VALIDATE_GE(eyeLineBottom, 0);
    VALIDATE_LT(faceHeight, photoHeight);

    if (crownTop <= 0 && eyeLineBottom > 0)
    {
        // Convert eye line to bottom distance into crown top distance
        constexpr const auto alpha = 0.477196;
        crownTop = photoHeight - eyeLineBottom - alpha * faceHeight;
    }

    VALIDATE_LT(crownTop, photoHeight - faceHeight);

    m_photoWidth = photoWidth;
    m_photoHeight = photoHeight;
    m_faceHeight = faceHeight;
    m_crownTop = crownTop;
    m_resolution_dpi = picResolution;
    m_units = units;
}

double PhotoStandard::photoWidth(const std::string & units) const
{
    return Utilities::convert(m_photoWidth, m_units, units, m_resolution_dpi);
}

double PhotoStandard::photoHeight(const std::string & units) const
{

    return Utilities::convert(m_photoHeight, m_units, units, m_resolution_dpi);
}

double PhotoStandard::faceHeight(const std::string & units) const
{
    return Utilities::convert(m_faceHeight, m_units, units, m_resolution_dpi);
}

double PhotoStandard::crownTop(const std::string & units) const
{
    return Utilities::convert(m_crownTop, m_units, units, m_resolution_dpi);
}

double PhotoStandard::resolutionDpi() const
{
    return m_resolution_dpi;
}

void PhotoStandard::overrideResolution(const double newDpi) const
{
    m_resolution_dpi = newDpi;
}

PhotoStandardSPtr PhotoStandard::fromJson(const std::string & photoStandardJson)
{
    rapidjson::Document d;
    d.Parse(photoStandardJson.c_str());
    return fromJson(d);
}

std::shared_ptr<PhotoStandard> PhotoStandard::fromJson(const rapidjson::Value & photoStandardJson)
{
    const auto photoHeight = photoStandardJson[PHOTO_HEIGHT].GetDouble();
    const auto photoWidth = photoStandardJson[PHOTO_WIDTH].GetDouble();
    const auto faceHeight = photoStandardJson[PHOTO_FACE_HEIGHT].GetDouble();
    const auto resolution = Utilities::getField(photoStandardJson, PHOTO_RESOLUTION, 300.0);
    const auto units = Utilities::getField(photoStandardJson, UNITS, std::string("mm"));
    const auto crownTop = Utilities::getField(photoStandardJson, PHOTO_CROWN_TOP, 0.0);
    const auto bottomEyeLine = Utilities::getField(photoStandardJson, PHOTO_EYELINE_BOTTOM, 0.0);

    return std::make_shared<PhotoStandard>(photoWidth,
                                           photoHeight,
                                           faceHeight,
                                           crownTop,
                                           bottomEyeLine,
                                           resolution,
                                           units);
}
} // namespace ppp

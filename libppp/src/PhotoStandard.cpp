#include "PhotoStandard.h"
#include "Utilities.h"

namespace ppp
{
PhotoStandard::PhotoStandard(const double picWidth,
                             const double picHeight,
                             const double faceHeight,
                             const double eyesHeight,
                             const std::string & units)
{
    m_picWidth_mm = Utilities::toMM(picWidth, units);
    m_picHeight_mm = Utilities::toMM(picHeight, units);
    m_faceHeight_mm = Utilities::toMM(faceHeight, units);
    m_eyesHeight_mm = Utilities::toMM(eyesHeight, units);
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

double PhotoStandard::eyesHeightMM() const
{
    return m_eyesHeight_mm;
}

std::shared_ptr<PhotoStandard> PhotoStandard::fromJson(rapidjson::Value & photoStandardJson)
{
    const auto picHeight = photoStandardJson["pictureHeight"].GetDouble();
    const auto picWidth = photoStandardJson["pictureWidth"].GetDouble();
    const auto faceHeight = photoStandardJson["faceHeight"].GetDouble();
    auto eyesHeight = 0.0;
    if (photoStandardJson.HasMember("eyesHeight"))
    {
        eyesHeight = photoStandardJson["eyesHeight"].GetDouble();
    }
    const auto units = photoStandardJson["units"].GetString();

    return std::make_shared<PhotoStandard>(picWidth, picHeight, faceHeight, eyesHeight, units);
}
} // namespace ppp

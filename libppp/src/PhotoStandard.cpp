#include "PhotoStandard.h"
#include "Utilities.h"

std::shared_ptr<PhotoStandard> PhotoStandard::fromJson(rapidjson::Value & psConfig)
{
    const auto picHeight = psConfig["pictureHeight"].GetDouble();
    const auto picWidth = psConfig["pictureWidth"].GetDouble();
    const auto faceLength = psConfig["faceHeight"].GetDouble();

    auto eyesHeight = 0.0;

    if (psConfig.HasMember("eyesHeight"))
    {
        eyesHeight = psConfig["eyesHeight"].GetDouble();
    }

    const auto units = psConfig["units"].GetString();

    return std::make_shared<PhotoStandard>(Utilities::toMM(picWidth, units),
                                           Utilities::toMM(picHeight, units),
                                           Utilities::toMM(faceLength, units),
                                           Utilities::toMM(eyesHeight, units));
}

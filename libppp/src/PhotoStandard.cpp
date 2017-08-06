#include "PhotoStandard.h"
#include "Utilities.h"


std::shared_ptr<PhotoStandard> PhotoStandard::fromJson(rapidjson::Value& psConfig)
{
    auto picHeight = psConfig["pictureHeight"].GetDouble();
    auto picWidth = psConfig["pictureWidth"].GetDouble();
    auto faceLength = psConfig["faceHeight"].GetDouble();

    auto eyesHeight = 0.0;

    if (psConfig.HasMember("eyesHeight"))
    {
        eyesHeight = psConfig["eyesHeight"].GetDouble();
    }

    auto units = psConfig["units"].GetString();

    return std::make_shared<PhotoStandard>(Utilities::toMM(picWidth, units),
                                           Utilities::toMM(picHeight, units),
                                           Utilities::toMM(faceLength, units),
                                           Utilities::toMM(eyesHeight, units));
}

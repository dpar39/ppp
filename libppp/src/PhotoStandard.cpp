#include "PhotoStandard.h"
#include <memory>


std::shared_ptr<PhotoStandard> PhotoStandard::fromJson(rapidjson::Value& psConfig)
{
    auto picHeight = psConfig["pictureHeight"].GetDouble();
    auto picWidth = psConfig["pictureWidth"].GetDouble();
    auto faceLength = psConfig["faceHeight"].GetDouble();
    auto units = psConfig["units"].GetString();

    double unitsRatio_mm;
    if (!strcmp(units,"inch"))
    {
        unitsRatio_mm = 25.4;
    }
    else if(!strcmp(units, "cm"))
    {
        unitsRatio_mm = 10.0;
    }
    else if (!strcmp(units, "mm"))
    {
        unitsRatio_mm = 1.0;
    }
    else
    {
        throw std::logic_error("Unknown input units when creating the photo standard definition");
    }

    return std::make_shared<PhotoStandard>(picWidth * unitsRatio_mm, picHeight * unitsRatio_mm, faceLength * unitsRatio_mm);
}

#include "PhotoStandard.h"
#include "Geometry.h"
#include <memory>
using namespace cv;

std::shared_ptr<PhotoStandard> PhotoStandard::fromJson(rapidjson::Value& psConfig)
{
    auto picHeight = psConfig["pictureHeight"].GetDouble();
    auto picWidth = psConfig["pictureWidth"].GetDouble();
    auto faceLength = psConfig["faceHeight"].GetDouble();
    auto units = psConfig["units"].GetString();

    double unitsRatio_mm;
    if (units == "inch")
    {
        unitsRatio_mm = 25.4;
    }
    else if (units == "cm")
    {
        unitsRatio_mm = 10.0;
    }
    else if (units == "mm")
    {
        unitsRatio_mm = 1.0;
    }
    else
    {
        throw std::logic_error("Unknown input units");
    }

    return std::make_shared<PhotoStandard>(picWidth * unitsRatio_mm, picHeight * unitsRatio_mm, faceLength * unitsRatio_mm);
}

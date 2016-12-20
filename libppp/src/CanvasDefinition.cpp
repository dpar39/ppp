#include "CanvasDefinition.h"

using namespace std;

CanvasDefinition::CanvasDefinition(double canvasWidth, double canvasHeight, double resolution, const std::string& units)
{
    m_border_mm = 1.0;
    double unitsRatio_mm;
    if (units == "inch")
    {
        unitsRatio_mm = 25.4;
    }
    else if (units == "cm")
    {
        unitsRatio_mm = 10;
    }
    else if (units == "mm")
    {
        unitsRatio_mm = 1;
    }
    else
    {
        throw std::runtime_error("Unknown input units when creating a print canvas definition");
    }

    m_canvasWidth_mm = canvasWidth * unitsRatio_mm;
    m_canvasHeight_mm = canvasHeight * unitsRatio_mm;
    m_resolution_ppmm = resolution / unitsRatio_mm;
    m_border_mm = 0.0;
}

std::shared_ptr<CanvasDefinition> CanvasDefinition::fromJson(rapidjson::Value& canvas)
{
    auto width = canvas["width"].GetFloat();
    auto height = canvas["height"].GetFloat();
    auto resolution = canvas["resolution"].GetFloat();
    auto units = canvas["units"].GetString();

    return std::make_shared<CanvasDefinition>(width, height, resolution, std::string(units));
}

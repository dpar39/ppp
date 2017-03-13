#include "CanvasDefinition.h"
#include "CommonHelpers.h"

using namespace std;

CanvasDefinition::CanvasDefinition(double canvasWidth, double canvasHeight, double resolution, const std::string& units)
{
    m_canvasWidth_mm = CommonHelpers::toMM(canvasWidth, units);
    m_canvasHeight_mm = CommonHelpers::toMM(canvasHeight, units);
    m_resolution_ppmm = CommonHelpers::toMM(resolution, units); 
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

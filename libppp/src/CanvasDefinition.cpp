#include "CanvasDefinition.h"
#include "Utilities.h"

using namespace std;

CanvasDefinition::CanvasDefinition(double canvasWidth, double canvasHeight, double dpi, const std::string & units)
{
    m_canvasWidth_mm = Utilities::toMM(canvasWidth, units);
    m_canvasHeight_mm = Utilities::toMM(canvasHeight, units);
    m_resolution_ppmm = dpi / Utilities::toMM(1, units);
    m_border_mm = 0.0;
}

double CanvasDefinition::height_mm() const
{
    return m_canvasHeight_mm;
}

double CanvasDefinition::width_mm() const
{
    return m_canvasWidth_mm;
}

double CanvasDefinition::resolution_ppmm() const
{
    return m_resolution_ppmm;
}

double CanvasDefinition::border() const
{
    return m_border_mm;
}

int CanvasDefinition::widthPixels() const
{
    return static_cast<int>(ceil(m_canvasWidth_mm * m_resolution_ppmm));
}

int CanvasDefinition::heightPixels() const
{
    return static_cast<int>(ceil(m_canvasHeight_mm * m_resolution_ppmm));
}

CanvasDefinitionSPtr CanvasDefinition::fromJson(rapidjson::Value & canvas)
{
    auto width = canvas["width"].GetFloat();
    auto height = canvas["height"].GetFloat();
    auto resolution = canvas["resolution"].GetFloat();
    const auto units = canvas["units"].GetString();

    return std::make_shared<CanvasDefinition>(width, height, resolution, std::string(units));
}

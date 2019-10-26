#include "CanvasDefinition.h"
#include "Utilities.h"

using namespace std;

CanvasDefinition::CanvasDefinition(const double width,
                                   const double height,
                                   const double dpi,
                                   const std::string & units,
                                   const double gutter,
                                   const double padding)
{
    m_canvasWidth_mm = Utilities::toMM(width, units);
    m_canvasHeight_mm = Utilities::toMM(height, units);
    m_resolution_ppmm = dpi / Utilities::toMM(1, units);
    m_gutter_mm = Utilities::toMM(gutter, units);
    m_padding_mm = Utilities::toMM(padding, units);
}

double CanvasDefinition::height() const
{
    return m_canvasHeight_mm;
}

double CanvasDefinition::width() const
{
    return m_canvasWidth_mm;
}

double CanvasDefinition::resolutionPixPerMM() const
{
    return m_resolution_ppmm;
}

double CanvasDefinition::border() const
{
    return m_gutter_mm;
}

double CanvasDefinition::padding() const
{
    return m_padding_mm;
}

int CanvasDefinition::widthPixels() const
{
    return ceilInteger((m_canvasWidth_mm + 2 * m_padding_mm) * m_resolution_ppmm);
}

int CanvasDefinition::heightPixels() const
{
    return ceilInteger((m_canvasHeight_mm + 2 * m_padding_mm) * m_resolution_ppmm);
}

int CanvasDefinition::gutterPixel() const
{
    return ceilInteger(m_gutter_mm * m_resolution_ppmm);
}

int CanvasDefinition::paddingPixels() const
{
    return ceilInteger(m_padding_mm * m_resolution_ppmm);
}

CanvasDefinitionSPtr CanvasDefinition::fromJson(rapidjson::Value & canvas)
{
    const auto width = canvas["width"].GetFloat();
    const auto height = canvas["height"].GetFloat();
    const auto resolution = canvas["resolution"].GetFloat();

    const auto getOptionalValue = [&canvas](const std::string & name, const double defaultValue = 0.0) {
        return canvas.HasMember(name) ? canvas[name].GetFloat() : defaultValue;
    };
    const auto padding = getOptionalValue("padding");
    const auto gutter = getOptionalValue("gutter", 0.0);
    const auto units = std::string(canvas["units"].GetString());
    const auto cd = std::make_shared<CanvasDefinition>(width, height, resolution, units, gutter, padding);
    return cd;
}

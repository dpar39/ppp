#include "PrintDefinition.h"
#include "Utilities.h"

namespace ppp
{
PrintDefinition::PrintDefinition(const double width,
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

double PrintDefinition::height() const
{
    return m_canvasHeight_mm;
}

double PrintDefinition::width() const
{
    return m_canvasWidth_mm;
}

double PrintDefinition::resolutionPixPerMM() const
{
    return m_resolution_ppmm;
}

double PrintDefinition::border() const
{
    return m_gutter_mm;
}

double PrintDefinition::padding() const
{
    return m_padding_mm;
}

int PrintDefinition::widthPixels() const
{
    return ceilInteger((m_canvasWidth_mm + 2 * m_padding_mm) * m_resolution_ppmm);
}

int PrintDefinition::heightPixels() const
{
    return ceilInteger((m_canvasHeight_mm + 2 * m_padding_mm) * m_resolution_ppmm);
}

int PrintDefinition::gutterPixel() const
{
    return ceilInteger(m_gutter_mm * m_resolution_ppmm);
}

int PrintDefinition::paddingPixels() const
{
    return ceilInteger(m_padding_mm * m_resolution_ppmm);
}

PrintDefinitionSPtr PrintDefinition::fromJson(rapidjson::Value & canvas)
{
    const auto width = Utilities::getField(canvas, PRINT_WIDTH, 0.0);
    const auto height = Utilities::getField(canvas, PRINT_HEIGHT, 0.0);
    const auto resolution = Utilities::getField(canvas, PRINT_RESOLUTION, 0.0);
    const auto padding = Utilities::getField(canvas, PRINT_PADDING, 0.0);
    const auto gutter = Utilities::getField(canvas, PRINT_GUTTER, 0.0);
    const auto units = Utilities::getField(canvas, UNITS, std::string("mm"));
    const auto cd = std::make_shared<PrintDefinition>(width, height, resolution, units, gutter, padding);
    return cd;
}
} // namespace ppp

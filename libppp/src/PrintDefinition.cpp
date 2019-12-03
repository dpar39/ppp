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
    VALIDATE_GT(width, 0);
    VALIDATE_GT(height, 0);
    VALIDATE_GE(gutter, 0);
    VALIDATE_GE(padding, 0);
    VALIDATE_LT(gutter, std::min(width, height));
    VALIDATE_LT(padding, std::min(width, height));
    m_canvasWidth = width;
    m_canvasHeight = height;
    m_resolution_dpi = dpi;
    m_gutter = gutter;
    m_padding = padding;
    m_units = units;
}

double PrintDefinition::height(const std::string & units) const
{
    return Utilities::convert(m_canvasHeight, m_units, units, m_resolution_dpi);
}

double PrintDefinition::width(const std::string & units) const
{
    return Utilities::convert(m_canvasWidth, m_units, units, m_resolution_dpi);
}

double PrintDefinition::gutter(const std::string & units) const
{
    return Utilities::convert(m_gutter, m_units, units, m_resolution_dpi);
}

double PrintDefinition::padding(const std::string & units) const
{
    return Utilities::convert(m_padding, m_units, units, m_resolution_dpi);
}

double PrintDefinition::totalWidth(const std::string & units) const
{
    return Utilities::convert(m_canvasWidth + 2 * m_padding, m_units, units, m_resolution_dpi);
}

double PrintDefinition::totalHeight(const std::string & units) const
{
    return Utilities::convert(m_canvasHeight + 2 * m_padding, m_units, units, m_resolution_dpi);
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

double PrintDefinition::resolutionDpi() const
{
    return m_resolution_dpi;
}

void PrintDefinition::overrideResolution(const double newDpi) const
{
    m_resolution_dpi = newDpi;
}
} // namespace ppp

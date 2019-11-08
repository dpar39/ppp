#pragma once

#include "CommonHelpers.h"
#include <rapidjson/document.h>
#include <string>

namespace cv
{
class Mat;
}

namespace ppp
{
class PhotoStandard;

FWD_DECL(CanvasDefinition)

class CanvasDefinition final
{
    double m_canvasWidth_mm { 0.0 }; ///<- Output canvas width in mm
    double m_canvasHeight_mm { 0.0 }; ///<- Output canvas height in mm
    double m_resolution_ppmm { 0.0 }; ///<- Resolution in pixels per mm
    double m_gutter_mm { 0.0 }; ///<- Separation between passport photos in the canvas in mm
    double m_padding_mm { 1.5 };

public:
    CanvasDefinition(double canvasWidth,
                     double canvasHeight,
                     double dpi,
                     const std::string & units,
                     double gutter = 0.0,
                     double padding = 0.0);

    double height() const;

    double width() const;

    double resolutionPixPerMM() const;

    double border() const;

    double padding() const;

    int widthPixels() const;

    int heightPixels() const;

    int gutterPixel() const;

    int paddingPixels() const;

    // canvas:
    // {
    //     height: 6,
    //     width: 4,
    //     resolution: 300,
    //     border: 0.1,
    //     units: "inch"
    // }
    /*!@brief Construct a CanvasDefinition from JSON data !*/
    static CanvasDefinitionSPtr fromJson(rapidjson::Value & canvas);
};
} // namespace ppp

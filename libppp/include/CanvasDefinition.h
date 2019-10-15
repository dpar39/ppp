#pragma once

#include "CommonHelpers.h"
#include <rapidjson/document.h>
#include <string>

class PhotoStandard;
namespace cv
{
class Mat;
}

FWD_DECL(CanvasDefinition)

class CanvasDefinition final
{
    double m_canvasWidth_mm; ///<- Output canvas width in mm
    double m_canvasHeight_mm; ///<- Output canvas height in mm
    double m_resolution_ppmm; ///<- Resolution in pixels per mm
    double m_border_mm; ///<- Separation between passport photos in the canvas in mm
public:
    CanvasDefinition(double canvasWidth, double canvasHeight, double dpi, const std::string & units = "mm");

    double height_mm() const;

    double width_mm() const;

    double resolution_ppmm() const;

    double border() const;

    int widthPixels() const;

    int heightPixels() const;

    // canvas:
    // {
    //     height: 6,
    //     width: 4,
    //     resolution: 330,
    //     border: 0.1,
    //     units: "inch"
    // }
    /*!@brief Construct a CanvasDefinition from JSON data !*/
    static CanvasDefinitionSPtr fromJson(rapidjson::Value & canvas);
};

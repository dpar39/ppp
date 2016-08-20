#pragma once
#include <string>
#include <memory>
#include <rapidjson/document.h>

class PhotoStandard;
namespace cv
{
    class Mat;
}

class CanvasDefinition
{
    double m_canvasWidth_mm;  ///<- Output canvas width in mm
    double m_canvasHeight_mm; ///<- Output canvas height in mm
    double m_resolution_ppmm; ///<- Resolution in pixels per mm
    double m_border_mm;  ///<- Separation between passport photos in the canvas in mm
public:
    CanvasDefinition(double canvasWidth, double canvasHeight, double resolution, const std::string &units = "mm");

    double height() const { return m_canvasHeight_mm; }
    double width() const { return m_canvasWidth_mm; }
    double resolution() const { return m_resolution_ppmm; }
    double border() const { return m_border_mm; }

    // canvas:
    // {
    //     height: 6,
    //     width: 4,
    //     resolution: 330,
    //     border: 0.1,
    //     units: "inch"
    // }
    /*!@brief Construct a CanvasDefinition from JSON data !*/
    static std::shared_ptr<CanvasDefinition> fromJson(rapidjson::Value& canvas);
};


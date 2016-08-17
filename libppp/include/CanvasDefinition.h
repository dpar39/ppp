#pragma once
#include <string>
#include <memory>
#include <rapidjson/document.h>

class PassportStandard;
namespace cv
{
    class Mat;
}

class CanvasDefinition
{
private:
    double m_canvasWidth_mm;  ///<- Output canvas width in mm
    double m_canvasHeight_mm; ///<- Output canvas height in mm
    double m_resolution_ppmm; ///<- Resolution in pixels per mm
    double m_border_mm;  ///<- Separation between passport photos in the canvas in mm
public:
    CanvasDefinition(double canvasWidth, double canvasHeight, double resolution, const std::string &units = "mm");

    cv::Mat tileCroppedPhoto(const PassportStandard &ps, const cv::Mat &croppedImage);

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


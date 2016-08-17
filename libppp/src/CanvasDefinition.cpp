#include "CanvasDefinition.h"
#include "PassportStandard.h"
#include "Geometry.h"

#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>


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
        throw std::runtime_error("Unknown input units");
    }

    m_canvasWidth_mm = canvasWidth * unitsRatio_mm;
    m_canvasHeight_mm = canvasHeight * unitsRatio_mm;
    m_resolution_ppmm = resolution / unitsRatio_mm;
    m_border_mm = 0.4;
}

cv::Mat CanvasDefinition::tileCroppedPhoto(const PassportStandard& ps, const cv::Mat& croppedImage)
{
    int canvasWidthPixels = ROUND_INT(m_resolution_ppmm*m_canvasWidth_mm);
    int canvasHeightPixels = ROUND_INT(m_resolution_ppmm*m_canvasHeight_mm);

    auto numPhotoRows = static_cast<size_t>(m_canvasHeight_mm / (ps.photoHeightMM() + m_border_mm));
    auto numPhotoCols = static_cast<size_t>(m_canvasWidth_mm / (ps.photoWidthMM() + m_border_mm));

    cv::Size tileSizePixels(ROUND_INT(m_resolution_ppmm * ps.photoWidthMM()), ROUND_INT(m_resolution_ppmm * ps.photoHeightMM()));

    // Resize input crop to the canvas output
    cv::Mat tileInCanvas;
    cv::resize(croppedImage, tileInCanvas, tileSizePixels);

    cv::Mat printPhoto(canvasHeightPixels, canvasWidthPixels, croppedImage.type());

    for (size_t row = 0; row < numPhotoRows; ++row)
    {
        for (size_t col = 0; col < numPhotoCols; ++col)
        {
            cv::Point topLeft(ROUND_INT(col*(ps.photoWidthMM() + m_border_mm)*m_resolution_ppmm),
                                                                                                ROUND_INT(row*(ps.photoHeightMM() + m_border_mm)*m_resolution_ppmm));
            tileInCanvas.copyTo(printPhoto(cv::Rect(topLeft, tileSizePixels)));
        }
    }
    return printPhoto;
}

std::shared_ptr<CanvasDefinition> CanvasDefinition::fromJson(rapidjson::Value& canvas)
{
    auto width = canvas["width"].GetFloat();
    auto height = canvas["height"].GetFloat();
    auto resolution = canvas["resolution"].GetFloat();
    auto units = canvas["units"].GetString();

    return std::make_shared<CanvasDefinition>(width, height, resolution, std::string(units));
}

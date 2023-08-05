
#pragma once

#include <opencv2/imgproc.hpp>
#include <string>

namespace ppp {
enum class VerticalAlignment
{
    BOTTOM,
    CENTER,
    TOP
};

enum class HorizontalAlignment
{
    LEFT,
    CENTER,
    RIGHT
};

class PhotoDecorator
{
public:
    static void drawText(const std::string & text,
                         cv::Rect boundingBox,
                         HorizontalAlignment hAlign,
                         VerticalAlignment vAlign,
                         cv::Mat & drawingImage,
                         cv::HersheyFonts fontFace = cv::HersheyFonts::FONT_HERSHEY_SIMPLEX,
                         double fontScale = 1.0f,
                         const cv::Scalar & textColor = cv::Scalar(255, 255, 255),
                         int fontThickness = 1,
                         bool autoPadding = true);
};
} // namespace ppp


#include "PhotoDecorator.h"

void PhotoDecorator::drawText(const std::string & text,
                              const cv::Rect boundingBox,
                              HorizontalAlignment hAlign,
                              VerticalAlignment vAlign,
                              cv::Mat & drawingImage,
                              const cv::HersheyFonts fontFace,
                              const double fontScale,
                              const cv::Scalar & textColor,
                              const int fontThickness,
                              bool autoPadding)
{
    auto x = boundingBox.x;
    auto y = boundingBox.y;

    auto baseline = 0;
    const auto textSize = getTextSize(text, fontFace, fontScale, fontThickness, &baseline);
    const auto centerPadding = (boundingBox.height - textSize.height) / 2;

    switch (vAlign)
    {
        case VerticalAlignment::BOTTOM:
            y += boundingBox.height - 1;
            break;
        case VerticalAlignment::CENTER:
            y += (boundingBox.height + textSize.height) / 2;
            break;
        case VerticalAlignment::TOP:
            y += textSize.height;
            break;
    }

    switch (hAlign)
    {
        case HorizontalAlignment::LEFT:
            x += 0;
            break;
        case HorizontalAlignment::CENTER:
            x += (boundingBox.width - textSize.width) / 2;
            break;
        case HorizontalAlignment::RIGHT:
            x += boundingBox.width - textSize.width;
            break;
    }

    if (vAlign == VerticalAlignment::CENTER && autoPadding)
    {
        switch (hAlign)
        {
            case HorizontalAlignment::LEFT:
                x += centerPadding;
                break;
            case HorizontalAlignment::RIGHT:
                x -= centerPadding;
                break;
            default:
                break;
        }
    }
    putText(drawingImage, text, cv::Point(x, y), fontFace, fontScale, textColor, fontThickness, cv::LINE_8, false);
}

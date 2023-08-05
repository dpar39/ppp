#include <gtest/gtest.h>

#include "PhotoDecorator.h"
#include <opencv2/imgcodecs.hpp>

using namespace cv;
namespace ppp {
TEST(PhotoDecorator, TestDrawTextWithAlignment)
{
    const auto canvasWidth = 640;
    const auto canvasHeight = 480;

    Mat canvas(canvasHeight, canvasWidth, CV_8UC3, Scalar(0, 0, 0));

    const Scalar redColor(0, 0, 255);
    const Scalar blueColor(255, 0, 0);

    const Rect boundingBox(static_cast<int>(canvasWidth * 0.05),
                           static_cast<int>(canvasHeight * 0.05),
                           static_cast<int>(canvasWidth * 0.9),
                           static_cast<int>(canvasHeight * 0.9));

    const auto bbCenter = Point(boundingBox.x + boundingBox.width / 2, boundingBox.y + boundingBox.height / 2);

    rectangle(canvas, boundingBox, redColor);

    line(canvas, Point(bbCenter.x, 0), Point(bbCenter.x, canvasWidth), blueColor);
    line(canvas, Point(0, bbCenter.y), Point(canvasWidth, bbCenter.y), blueColor);

    const auto textNames = std::vector<std::string> {
        "LEFT_BOTTOM", "LEFT_CENTER",  "LEFT_TOP",     "CENTER_BOTTOM", "CENTER_CENTER",
        "CENTER_TOP",  "RIGHT_BOTTOM", "RIGHT_CENTER", "RIGHT_TOP",
    };

    auto i = 0;
    for (const auto hAlign : { HorizontalAlignment::LEFT, HorizontalAlignment::CENTER, HorizontalAlignment::RIGHT }) {
        for (const auto vAlign : { VerticalAlignment::BOTTOM, VerticalAlignment::CENTER, VerticalAlignment::TOP }) {
            const auto text = textNames[i++];
            PhotoDecorator::drawText(text,
                                     boundingBox,
                                     hAlign,
                                     vAlign,
                                     canvas,
                                     FONT_HERSHEY_SIMPLEX,
                                     0.75,
                                     redColor,
                                     1,
                                     false);
        }
    }

    // imwrite("test.png", canvas);
}
} // namespace ppp

#include <LandMarks.h>
#include <gtest/gtest.h>
#include <memory>

#include "ComplianceChecker.h"
#include "ComplianceResult.h"
#include "ImageStore.h"
#include "PhotoStandard.h"
#include "SelfieSegmentation.h"
#include "TestHelpers.h"

#include "FaceMeshExtractor.h"

#include <opencv2/core/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace testing;
using namespace cv;

namespace ppp {
class SegmentationTests : public Test
{ };

TEST_F(SegmentationTests, selfieSegmentationTest)
{
   // const auto imagePath = resolvePath("research/my_database/DSCF0242.JPG");
    // const auto imagePath = resolvePath("research/001_partTL.png");
    // auto imageStore = std::make_unique<ImageStore>();
    // auto imageKey = imageStore->setImage(imagePath);
    // auto inputImage = imageStore->getImage(imageKey);

    // auto faceSegmentation = SelfieSegmentation();

    // faceSegmentation.configure(nullptr);
    // auto alpha1 = faceSegmentation.computeSegmentation(inputImage);

    // auto bgColor = Scalar(255.f, 0.f, 10.f);
    // // auto bgColor = Scalar(255.f, 255.f, 255.f);
    // auto background = cv::Mat(inputImage.size(), CV_32FC3, bgColor);

    // cv::Mat alpha;
    // cv::cvtColor(alpha1, alpha, cv::COLOR_GRAY2BGR, 3);

    // Mat foreground = inputImage.clone();
    // foreground.convertTo(foreground, CV_32FC3);

    // // Storage for output image
    // Mat ouImage = Mat::zeros(foreground.size(), foreground.type());

    // // Multiply the foreground with the alpha matte
    // multiply(alpha, foreground, foreground);

    // // Multiply the background with ( 1 - alpha )
    // multiply(Scalar::all(1.0) - alpha, background, background);

    // // Add the masked foreground and background.
    // add(foreground, background, ouImage);

    // cv::imwrite("tmp/p1.png", ouImage);
}

} // namespace ppp

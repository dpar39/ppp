#include <gtest/gtest.h>

#include <opencv2/imgcodecs.hpp>

#include "PhotoStandard.h"
#include "CanvasDefinition.h"
#include "PhotoPrintMaker.h"

#include "TestHelpers.h"

using namespace std;

#define MANUAL_CHECK 0

class PhotoPrintMakerTests : public ::testing::Test
{
protected:
    PhotoPrintMakerSPtr m_pPhotoPrintMaker = make_shared<PhotoPrintMaker>();
};

void verifyEqualImage(const std::string &expectedImageFilePath, const cv::Mat &actualImage)
{
    auto expectedImage = cv::imread(expectedImageFilePath);
    auto numDisctintPixels = cv::countNonZero(cv::sum(cv::abs(expectedImage - actualImage)));
    EXPECT_EQ(numDisctintPixels, 0) << "Actual image differs to image in file " << expectedImageFilePath;
}


TEST_F(PhotoPrintMakerTests, TestCroppingWorks)
{
    PhotoStandard passportStandard(35.0, 45.0, 34.0);
    CanvasDefinition canvasDefinition(6, 4, 300, "inch");

    const auto& imageFileName = resolvePath("research/sample_test_images/000.jpg");
    cv::Point2d crownPos(941, 999);
    cv::Point2d chinPos(927, 1675);

    auto image = cv::imread(imageFileName);

    // Crop the photo to the right dimensions
    auto croppedImage = m_pPhotoPrintMaker->cropPicture(image, crownPos, chinPos, passportStandard);
    
    auto expectedCropPath = pathCombine(resolvePath("libppp/test/data"), "000-cropped.png");
#if MANUAL_CHECK  // Set to 1 for manual check
    cv::imwrite(expectedCropPath, croppedImage);
#else
    verifyEqualImage(expectedCropPath, croppedImage);
#endif

    // Draw tiles into the printing canvas
    auto printPhoto = m_pPhotoPrintMaker->tileCroppedPhoto(canvasDefinition, passportStandard, croppedImage);

    auto expectedPrintPath = pathCombine(resolvePath("libppp/test/data"), "000-print.png");
#if MANUAL_CHECK  // Set to 1 for manual check
    cv::imwrite(expectedPrintPath, printPhoto);
#else
    verifyEqualImage(expectedPrintPath, printPhoto);
#endif

}
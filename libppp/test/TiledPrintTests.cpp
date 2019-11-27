#include <gtest/gtest.h>

#include <opencv2/imgcodecs.hpp>

#include "CanvasDefinition.h"
#include "PhotoPrintMaker.h"
#include "PhotoStandard.h"

#include "TestHelpers.h"

using namespace std;

#define MANUAL_CHECK 0

namespace ppp
{
class PhotoPrintMakerTests : public testing::Test
{
protected:
    PhotoPrintMakerSPtr m_pPhotoPrintMaker = make_shared<PhotoPrintMaker>();
};

void verifyEqualImage(const std::string & expectedImageFilePath, const cv::Mat & actualImage)
{

    const auto expectedImage = cv::imread(expectedImageFilePath);
    const auto numDisctintPixels = countNonZero(sum(cv::abs(expectedImage - actualImage)));
    EXPECT_LE(numDisctintPixels, 3) << "Actual image differs to image in file " << expectedImageFilePath;
}

TEST_F(PhotoPrintMakerTests, TestCroppingWorks)
{
    const PhotoStandard passportStandard(35.0, 45.0, 34.0);
    const CanvasDefinition canvasDefinition(6, 4, 300, "inch");

    const auto & imageFileName = resolvePath("research/sample_test_images/000.jpg");
    const cv::Point2d crownPos(941, 999);
    const cv::Point2d chinPos(927, 1675);

    const auto image = cv::imread(imageFileName);

    // Crop the photo to the right dimensions
    const auto croppedImage = m_pPhotoPrintMaker->cropPicture(image, crownPos, chinPos, passportStandard);
    benchmarkValidate(croppedImage, "_croppedPhoto");

    // Draw tiles into the printing canvas
    const auto tiledPhoto = m_pPhotoPrintMaker->tileCroppedPhoto(canvasDefinition, passportStandard, croppedImage);
    benchmarkValidate(tiledPhoto, "_tiledPhoto");
}

TEST_F(PhotoPrintMakerTests, TestCroppingWorksWithPadding)
{
    const PhotoStandard passportStandard(2, 2, 19.0 / 16.0, 0.0, "inch");
    const CanvasDefinition canvasDefinition(6, 4, 300, "inch", 0, 1.5 / 25.4);

    const auto & imageFileName = resolvePath("research/my_database/20191021_155155.jpg");
    const cv::Point2d crownPos(1155, 310);
    const cv::Point2d chinPos(1173, 1188);

    const auto image = cv::imread(imageFileName);

    // Crop the photo to the right dimensions
    const auto croppedImage = m_pPhotoPrintMaker->cropPicture(image, crownPos, chinPos, passportStandard);

    const auto printPhoto = m_pPhotoPrintMaker->tileCroppedPhoto(canvasDefinition, passportStandard, croppedImage);
    benchmarkValidate(printPhoto);
}
} // namespace ppp

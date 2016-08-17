#include <gtest/gtest.h>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>

#include "PassportStandard.h"
#include "CanvasDefinition.h"
#include "TestHelpers.h"
#include <common.h>

using namespace std;

TEST(TiledPrintTest, TestCroppingWorks)
{
    PassportStandard passportStandard(35.0, 45.0, 34.0);

    CanvasDefinition canvasDefinition(6, 4, 300, "inch");

    string imageDir = resolvePath("research/sample_test_images");
    imageDir = resolvePath("research/mugshot_frontal_original_all");
    vector<string> imageFileNames;
    getImageFiles(imageDir, imageFileNames);

    for (const auto& imageFileName : imageFileNames)
    {
        string landMarkFiles = imageFileName.substr(0, imageFileName.find_last_of('.')) + ".pos";
        cv::Mat landMarks;
        ASSERT_TRUE(importTextMatrix(landMarkFiles, landMarks));

        cv::Point2d crownPos(landMarks.at<float>(0, 0), landMarks.at<float>(0, 1));
        cv::Point2d chinPos(landMarks.at<float>(16, 0), landMarks.at<float>(16, 1));
        cv::Mat image = cv::imread(imageFileName);

        cv::Mat croppedImage = passportStandard.cropPicture(image, crownPos, chinPos);

        // Draw tiles
        cv::Mat printPhoto = canvasDefinition.tileCroppedPhoto(passportStandard, croppedImage);
    }

}

TEST(TiledPrintTest, TestForDarien1)
{
    string imageDir = resolvePath("research/sample_test_images");

    PassportStandard passportStandard(35.0, 45.0, 34.0);
    CanvasDefinition canvasDefinition(6, 4, 300, "inch");

    auto imageFileName = imageDir + "/2015-02-01 14.41.19.jpg";
    cv::Point2d crownPos(953, 1018), chinPos(927,1683);

    cv::Mat image = cv::imread(imageFileName);

    cv::Mat croppedImage = passportStandard.cropPicture(image, crownPos, chinPos);

    // Draw tiles
    cv::Mat printPhoto = canvasDefinition.tileCroppedPhoto(passportStandard, croppedImage);

    cv::imwrite(imageDir + "/output.png", printPhoto);
}

TEST(TiledPrintTest, TestForDarien)
{
    auto imageDir = resolvePath("research/sample_test_images");

    PassportStandard passportStandard(35.0, 45.0, 34.0);
    CanvasDefinition canvasDefinition(6, 4, 300, "inch");

    auto imageFileName = imageDir + "/98506e2f-d374-4e7a-99d0-702a721f8bb8.jpg";
    cv::Point2d crownPos(299, 150), chinPos(299, 670);

    auto image = cv::imread(imageFileName);

    auto croppedImage = passportStandard.cropPicture(image, crownPos, chinPos);

    // Draw tiles
    cv::Mat printPhoto = canvasDefinition.tileCroppedPhoto(passportStandard, croppedImage);

    cv::imwrite(imageDir + "/output-loira.png", printPhoto);
}
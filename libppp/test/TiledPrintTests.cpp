#include <gtest/gtest.h>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>

#include "PhotoStandard.h"
#include "CanvasDefinition.h"
#include "PhotoPrintMaker.h"

#include "TestHelpers.h"

using namespace std;

TEST(TiledPrintTest, TestCroppingWorks)
{
    PhotoStandard passportStandard(35.0, 45.0, 34.0);

    CanvasDefinition canvasDefinition(6, 4, 300, "inch");

    auto imageDir = resolvePath("research/mugshot_frontal_original_all");
    vector<string> imageFileNames;
    getImageFiles(imageDir, imageFileNames);

    for (const auto& imageFileName : imageFileNames)
    {
        auto landMarkFiles = imageFileName.substr(0, imageFileName.find_last_of('.')) + ".pos";
        cv::Mat landMarks;
        ASSERT_TRUE(importTextMatrix(landMarkFiles, landMarks));

        cv::Point2d crownPos(landMarks.at<float>(0, 0), landMarks.at<float>(0, 1));
        cv::Point2d chinPos(landMarks.at<float>(16, 0), landMarks.at<float>(16, 1));
        auto image = cv::imread(imageFileName);

        PhotoPrintMaker maker;
        // Crop the picture
        auto croppedImage = maker.cropPicture(image, crownPos, chinPos, passportStandard);
        // Draw tiles into the printing canvas
        auto printPhoto = maker.tileCroppedPhoto(canvasDefinition, passportStandard, croppedImage);
    }
}
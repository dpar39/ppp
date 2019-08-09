#include "FaceDetector.h"
#include "TestHelpers.h"
#include "Utilities.h"
#include <gtest/gtest.h>

#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

class FaceDetectorTests : public testing::Test
{
protected:
    FaceDetectorSPtr m_pFaceDetector;

    void SetUp() override
    {
        m_pFaceDetector = std::make_shared<FaceDetector>();
        std::string configString;
        readConfigFromFile("", configString);

        rapidjson::Document config;
        config.Parse(configString.c_str());

        m_pFaceDetector->configure(config);
    }
};

TEST_F(FaceDetectorTests, DISABLED_CanDetectFaces)
{
    const auto process = [&](const std::string & imagePrefix,
                             cv::Mat & rgbImage,
                             cv::Mat & grayImage,
                             const LandMarks & manualAnnotations,
                             LandMarks & detectedLandMarks) -> bool {
        EXPECT_TRUE(m_pFaceDetector->detectLandMarks(grayImage, detectedLandMarks))
            << "Error detecting face in " << imagePrefix;

        // Check that the rectangle contains both eyes and mouth points
        const auto faceRect = detectedLandMarks.vjFaceRect;

        EXPECT_TRUE(IN_ROI(faceRect, manualAnnotations.eyeLeftPupil));
        EXPECT_TRUE(IN_ROI(faceRect, manualAnnotations.eyeRightPupil));
        EXPECT_TRUE(IN_ROI(faceRect, manualAnnotations.lipLeftCorner));
        EXPECT_TRUE(IN_ROI(faceRect, manualAnnotations.lipRightCorner));

        return true;
    };

    std::vector<ResultData> rd;
    processDatabase(process,
                    std::vector<std::string>(),
                    "research/mugshot_frontal_original_all/via_region_data_dpd.csv",
                    rd);
}

TEST_F(FaceDetectorTests, DISABLED_DetectFaceRotation)
{
    const auto imageFileName = resolvePath("research/my_database/000.jpg");
    auto inputImage = cv::imread(imageFileName);
    cv::Mat grayImage;
    cv::cvtColor(inputImage, grayImage, cv::COLOR_BGR2GRAY);

    for (const auto angle : { 0, 90, -90, 180 })
    {
        const auto rotatedImage = Utilities::rotateImage(inputImage, angle);

        LandMarks detectedLandMarks;
        EXPECT_TRUE(m_pFaceDetector->detectLandMarks(rotatedImage, detectedLandMarks))
            << "Unable to detect a face in this image";

        const auto angleSum = angle + detectedLandMarks.imageRotation;
        EXPECT_TRUE(angleSum == 0 || angleSum == 360);
    }
}
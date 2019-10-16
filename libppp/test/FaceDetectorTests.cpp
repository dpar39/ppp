#include "FaceDetector.h"
#include "TestHelpers.h"
#include "Utilities.h"
#include <gtest/gtest.h>

#include "ImageStore.h"
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

    const auto inRoi = [](const cv::Rect & r, const cv::Point & p) -> bool {
        return p.x > r.x && p.x < r.x + r.width && p.y > r.y && p.y < r.y + r.height;
    };
    const auto imageStore = std::make_shared<ImageStore>();
    const auto process = [&](const std::string & imageFilePath,
                             const LandMarks & manualAnnotations,
                             LandMarks & detectedLandMarks) -> std::pair<bool, cv::Mat> {
        const auto imageKey = imageStore->setImage(imageFilePath);
        const auto rgbImage = imageStore->getImage(imageKey);

        EXPECT_TRUE(m_pFaceDetector->detectLandMarks(rgbImage, detectedLandMarks))
            << "Error detecting face in " << imageFilePath;

        // Check that the rectangle contains both eyes and mouth points
        const auto faceRect = detectedLandMarks.vjFaceRect;

        EXPECT_TRUE(inRoi(faceRect, manualAnnotations.eyeLeftPupil));
        EXPECT_TRUE(inRoi(faceRect, manualAnnotations.eyeRightPupil));
        EXPECT_TRUE(inRoi(faceRect, manualAnnotations.lipLeftCorner));
        EXPECT_TRUE(inRoi(faceRect, manualAnnotations.lipRightCorner));

        return { true, rgbImage };
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
    const auto inputImage = cv::imread(imageFileName);
    cv::Mat grayImage;
    cv::cvtColor(inputImage, grayImage, cv::COLOR_BGR2GRAY);

    for (const auto angle : { 0, 90, -90, 180 })
    {
        const auto rotatedImage = Utilities::rotateImage(inputImage, angle);

        LandMarks detectedLandMarks;
        EXPECT_TRUE(m_pFaceDetector->detectLandMarks(rotatedImage, detectedLandMarks))
            << "Unable to detect a face in this image";

        const auto angleSum = angle + detectedLandMarks.imageRotation;
        EXPECT_EQ(detectedLandMarks.imageRotation, angle);
    }
}

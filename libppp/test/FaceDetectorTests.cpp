#include <gtest/gtest.h>
#include "FaceDetector.h"
#include "TestHelpers.h"

class FaceDetectorTests : public ::testing::Test
{
protected:
    FaceDetectorSPtr m_pFaceDetector;

    void SetUp() override
    {
        m_pFaceDetector = std::make_shared<FaceDetector>();
        auto config = readConfigFromFile();
        m_pFaceDetector->configure(config);
    }
};

TEST_F(FaceDetectorTests, CanDetectFaces)
{
    auto process = [&](const std::string& imagePrefix, cv::Mat& rgbImage, cv::Mat& grayImage,
        const LandMarks& manualAnnotations, LandMarks& detectedLandMarks) -> bool
    {
        EXPECT_TRUE(m_pFaceDetector->detectLandMarks(grayImage, detectedLandMarks))
            << "Error detecting face in " << imagePrefix;

        // Check that the rectangle contains both eyes and mouth points
        auto faceRect = detectedLandMarks.vjFaceRect;

        EXPECT_TRUE(IN_ROI(faceRect, manualAnnotations.eyeLeftPupil));
        EXPECT_TRUE(IN_ROI(faceRect, manualAnnotations.eyeRightPupil));
        EXPECT_TRUE(IN_ROI(faceRect, manualAnnotations.lipLeftCorner));
        EXPECT_TRUE(IN_ROI(faceRect, manualAnnotations.lipRightCorner));

        return true;
    };

    ResultsData rd;
    processDatabase(process, std::vector<std::string>(), "research/mugshot_frontal_original_all/via_region_data_dpd.csv", rd);
}
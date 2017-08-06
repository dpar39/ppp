#include <gtest/gtest.h>
#include <vector>


#include <PppEngine.h>
#include <LandMarks.h>
#include <Utilities.h>

#include "TestHelpers.h"

#include <FaceDetector.h>
#include <EyeDetector.h>

using namespace cv;

class PppEngineIntegrationTests : public ::testing::Test
{
protected:
    PppEngineSPtr m_pPppEngine = std::make_shared<PppEngine>();

    void SetUp() override
    {
        rapidjson::Document config;
        EXPECT_NO_THROW(config = readConfigFromFile());
        EXPECT_NO_THROW(m_pPppEngine->configure(config));
    }
};


TEST_F(PppEngineIntegrationTests, EndToEndDetectioWorks)
{
    std::vector<double> leftEyeErrors, rightEyeErrors;
    std::vector<double> leftLipsErrors, rightLipsErrors;
    std::vector<double> relativeErrors;

    auto process = [&](const std::string& imagePrefix, cv::Mat& rgbImage, cv::Mat& grayImage,
            const LandMarks& annotations, LandMarks& detectedLandMarks) -> bool
        {
            auto imgKey = m_pPppEngine->setInputImage(rgbImage);

            auto success = m_pPppEngine->detectLandMarks(imgKey, detectedLandMarks);
            EXPECT_TRUE(success) << "Error detecting landmarks in " << imagePrefix;

            success = IN_ROI(detectedLandMarks.vjLeftEyeRect, annotations.eyeLeftPupil) &&
                IN_ROI(detectedLandMarks.vjRightEyeRect, annotations.eyeRightPupil);

            EXPECT_TRUE(success) << "Failed " << imagePrefix << std::endl;

            double maxEyeAllowedError = 16;
            auto leftEyeError = cv::norm(detectedLandMarks.eyeLeftPupil - annotations.eyeLeftPupil);
            auto rightEyeError = cv::norm(detectedLandMarks.eyeRightPupil - annotations.eyeRightPupil);

            double maxLipCornerAllowedError = 25;
            auto leftLipsError = cv::norm(detectedLandMarks.lipLeftCorner - annotations.lipLeftCorner);
            auto rightLipsError = cv::norm(detectedLandMarks.lipRightCorner - annotations.lipRightCorner);

            leftEyeErrors.push_back(leftEyeError);
            rightEyeErrors.push_back(rightEyeError);
            leftLipsErrors.push_back(leftLipsError);
            rightLipsErrors.push_back(rightLipsError);

            if (leftEyeError > maxEyeAllowedError)
            {
                std::cout << " Problem detecting left eye in " << imagePrefix << std::endl;
            }
            if (rightEyeError > maxEyeAllowedError)
            {
                std::cout << " Problem detecting right eye in " << imagePrefix << std::endl;
            }

            if (leftLipsError > maxLipCornerAllowedError)
            {
                std::cout << " Problem detecting left lips corner in " << imagePrefix << std::endl;
            }
            if (rightLipsError > maxLipCornerAllowedError)
            {
                std::cout << " Problem detecting right lips corner in " << imagePrefix << std::endl;
            }

            // Validate chin-crown distance error
            auto expectedDistance = cv::norm(annotations.chinPoint - annotations.crownPoint);
            auto actualDistance = cv::norm(detectedLandMarks.chinPoint - detectedLandMarks.crownPoint);

            const auto maxError = (36.0 - 32.0) / 34.0;
            auto relError = cv::abs(expectedDistance - actualDistance) / expectedDistance;
            relativeErrors.push_back(relError);
            auto accepted = relError < maxError;

            if (!accepted)
            {
                std::cout << " Estimation of face height too large for image " << imagePrefix << std::endl;
            }
            return accepted;

            return true;
        };

    std::vector<int> excludeList = {
        //1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,
        //56, // Glasses with some reflexion
        74, // Eyes totally closed
        81, // red bear problem
        //76, // Glasses with some reflexion
        //88, // Glasses, eyes mostly closed
        //92, // Right eye fail <<<<<<<<<
        //115, // Old guy, eyes very closed
        //121
    };

    std::vector<std::string> ignoreImageList;
    transform(excludeList.begin(), excludeList.end(), std::back_inserter(ignoreImageList),
        [](int i)
    {
        std::stringstream ss;
        ss << std::setw(3) << std::setfill('0') << i;
        return ss.str() + "_frontal.jpg";
    });

    ResultsData rd;
    processDatabase(process, ignoreImageList, "research/mugshot_frontal_original_all/via_region_data_dpd.csv", rd);
}

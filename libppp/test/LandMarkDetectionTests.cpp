#include <gtest/gtest.h>
#include <vector>


#include <PppEngine.h>
#include <LandMarks.h>
#include <Utilities.h>

#include "TestHelpers.h"

#include <FaceDetector.h>
#include <numeric>
#include <opencv2/imgcodecs.hpp>

using namespace cv;

class Stats
{
public:
    double medianValue;
    double meanValue;
    double stdDevValue;
    double minValue;
    double maxValue;
    size_t minIndex;
    size_t maxIndex;

    explicit Stats(const std::vector<double> &values)
        : medianValue(0)
        , meanValue(0)
        , stdDevValue(0)
        , minValue(0)
        , maxValue(0)
    {
        if (values.empty())
        {
            return;
        }

        auto n = values.size();

        auto itpair = std::minmax_element(values.begin(), values.end());
        minValue = *itpair.first;
        maxValue = *itpair.second;
        minIndex = std::distance(values.begin(), itpair.first);
        maxIndex = std::distance(values.begin(), itpair.second);

        medianValue = median(values);
        meanValue = std::accumulate(values.begin(), values.end(), 0.0) / (n + std::numeric_limits<double>::epsilon());
        stdDevValue = sqrt(std::accumulate(values.begin(), values.end(), 0.0, [this](double init, double v) { return init + (v - meanValue)*(v - meanValue); }) / n);
    }

    friend std::ostream & operator<<(std::ostream &os, const Stats &s)
    {
        os << std::setprecision(5) << "{mean: " << s.minValue << ", stddev: " << s.stdDevValue << ", median: " << s.medianValue
            << ", min: " << s.minValue << " (@" << s.minIndex << "), max: " << s.maxValue << " (@" << s.maxIndex << ")}";
        return os;
    }
};

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

    void processResults(const std::vector<ResultData> &resultsData) const
    {
        std::vector<double> crownChinEstimationErrors, scalingErrors;
        crownChinEstimationErrors.reserve(resultsData.size());
        scalingErrors.reserve(resultsData.size());
        for (const auto &r : resultsData)
        {
            EXPECT_TRUE(r.isSuccess);

            if (!r.isSuccess)
            {
                continue;
            }

            auto errCrown = cv::norm(r.annotation.crownPoint - r.detection.crownPoint);
            auto errChin = cv::norm(r.annotation.chinPoint - r.detection.chinPoint);
            auto crownChinActualDist = cv::norm(r.annotation.chinPoint - r.annotation.crownPoint);
            auto crownChinEstimatedDist = cv::norm(r.detection.chinPoint - r.detection.crownPoint);

            crownChinEstimationErrors.push_back((errCrown + errChin) / crownChinActualDist);
            scalingErrors.push_back(abs(crownChinEstimatedDist - crownChinActualDist) / crownChinActualDist);
        }

        Stats s(scalingErrors); Stats crownChinStats(crownChinEstimationErrors);
        std::cout << "Scale errors" << s << std::endl;
        std::cout << "Crown-Chin estimation relative error: " << crownChinStats << std::endl;
    }

    bool runLandMarkDetection(const cv::Mat &rgbImage, LandMarks& detectedLandMarks) const
    {
        auto imgKey = m_pPppEngine->setInputImage(rgbImage);
        return m_pPppEngine->detectLandMarks(imgKey, detectedLandMarks);
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

            auto success = runLandMarkDetection(rgbImage, detectedLandMarks);
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

    std::vector<ResultData> rd;

    processDatabase(process, ignoreImageList, "research/mugshot_frontal_original_all/via_region_data_dpd.csv", rd);

    processResults(rd);
}


TEST_F(PppEngineIntegrationTests, DevelopementTestSingleCase)
{
    auto imageFileName = resolvePath("research/mugshot_frontal_original_all/071_frontal.jpg");
    auto inputImage = cv::imread(imageFileName);

    LandMarks detectedLandMarks;
    auto success = runLandMarkDetection(inputImage, detectedLandMarks);

    EXPECT_TRUE(success) << "Failed to process image " << imageFileName;

}
#include <gtest/gtest.h>
#include <vector>

#include <LandMarks.h>
#include <PppEngine.h>
#include <Utilities.h>

#include "TestHelpers.h"

#include <FaceDetector.h>
#include <numeric>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace cv;

std::string pct(const double v)
{
    return std::to_string(v * 100) + "%";
}

class Stats
{
public:
    double medianValue = 0;
    double meanValue = 0;
    double stdDevValue = 0;
    double minValue = 0;
    double maxValue = 0;
    size_t minIndex = 0;
    size_t maxIndex = 0;

    explicit Stats(const std::vector<double> & values)
    {
        if (values.empty())
        {
            return;
        }

        const auto n = values.size();

        const auto itpair = std::minmax_element(values.begin(), values.end());
        minValue = *itpair.first;
        maxValue = *itpair.second;
        minIndex = std::distance(values.begin(), itpair.first);
        maxIndex = std::distance(values.begin(), itpair.second);

        medianValue = median(values);
        meanValue = std::accumulate(values.begin(), values.end(), 0.0) / (n + std::numeric_limits<double>::epsilon());
        stdDevValue = sqrt(std::accumulate(values.begin(),
                                           values.end(),
                                           0.0,
                                           [this](const double init, const double v) {
                                               return init + (v - meanValue) * (v - meanValue);
                                           })
                           / n);
    }

    friend std::ostream & operator<<(std::ostream & os, const Stats & s)
    {
        os << std::setprecision(5) << "{mean: " << pct(s.meanValue) << ", stddev: " << pct(s.stdDevValue)
           << ", median: " << pct(s.medianValue) << ", min: " << pct(s.minValue) << " (@" << s.minIndex
           << "), max: " << pct(s.maxValue) << " (@" << s.maxIndex << ")}";
        return os;
    }
};

class PppEngineIntegrationTests : public testing::Test
{
protected:
    PppEngineSPtr m_pPppEngine = std::make_shared<PppEngine>();

    void SetUp() override
    {
        rapidjson::Document config;
        EXPECT_NO_THROW(config = readConfigFromFile());
        EXPECT_NO_THROW(m_pPppEngine->configure(config));
    }

    void processResults(const std::vector<ResultData> & resultsData) const
    {
        std::vector<double> crownChinEstimationErrors, scalingErrors;
        crownChinEstimationErrors.reserve(resultsData.size());
        scalingErrors.reserve(resultsData.size());
        for (const auto & r : resultsData)
        {
            EXPECT_TRUE(r.isSuccess);

            if (!r.isSuccess)
            {
                continue;
            }

            const auto errCrown = norm(r.annotation.crownPoint - r.detection.crownPoint);
            const auto errChin = norm(r.annotation.chinPoint - r.detection.chinPoint);
            const auto crownChinActualDist = norm(r.annotation.chinPoint - r.annotation.crownPoint);
            const auto crownChinEstimatedDist = norm(r.detection.chinPoint - r.detection.crownPoint);

            crownChinEstimationErrors.push_back((errCrown + errChin) / crownChinActualDist);
            scalingErrors.push_back(abs(crownChinEstimatedDist - crownChinActualDist) / crownChinActualDist);
        }

        const Stats s(scalingErrors);
        const Stats crownChinStats(crownChinEstimationErrors);
        std::cout << "Scale errors" << s << std::endl;
        std::cout << "Crown-Chin estimation relative error: " << crownChinStats << std::endl;
    }

    bool runLandMarkDetection(const Mat & rgbImage, LandMarks & detectedLandMarks) const
    {
        const auto imgKey = m_pPppEngine->setInputImage(rgbImage);
        return m_pPppEngine->detectLandMarks(imgKey, detectedLandMarks);
    }
};

TEST_F(PppEngineIntegrationTests, EndToEndDetectioWorks)
{
    std::vector<double> leftEyeErrors, rightEyeErrors;
    std::vector<double> leftLipsErrors, rightLipsErrors;
    std::vector<double> relativeErrors;

    const auto process = [&](const std::string & imagePrefix,
                             Mat & rgbImage,
                             Mat & grayImage,
                             const LandMarks & annotations,
                             LandMarks & detectedLandMarks) -> bool {
        auto imgKey = m_pPppEngine->setInputImage(rgbImage);

        const auto success = runLandMarkDetection(rgbImage, detectedLandMarks);
        EXPECT_TRUE(success) << "Error detecting landmarks in " << imagePrefix;

        const double maxEyeAllowedError = 25;
        const auto leftEyeError = norm(detectedLandMarks.eyeLeftPupil - annotations.eyeLeftPupil);
        const auto rightEyeError = norm(detectedLandMarks.eyeRightPupil - annotations.eyeRightPupil);

        const double maxLipCornerAllowedError = 25;
        const auto leftLipsError = norm(detectedLandMarks.lipLeftCorner - annotations.lipLeftCorner);
        const auto rightLipsError = norm(detectedLandMarks.lipRightCorner - annotations.lipRightCorner);

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
        const auto expectedDistance = norm(annotations.chinPoint - annotations.crownPoint);
        const auto actualDistance = norm(detectedLandMarks.chinPoint - detectedLandMarks.crownPoint);

        const auto maxError = (36.0 - 32.0) / 34.0;
        const auto relError = abs(expectedDistance - actualDistance) / expectedDistance;
        const auto accepted = relError < maxError;

        if (!accepted)
        {
            std::cout << " *** Estimation of face height too large for image " << imagePrefix << std::endl;
        }
        return accepted;
    };

    std::vector<int> excludeList = {
        // 1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,
        12,
        // 16,
        // 56, // Glasses with some reflexion
        74, // Eyes totally closed
        // 81, // red bear problem
        // 76, // Glasses with some reflexion
        // 88, // Glasses, eyes mostly closed
        // 92, // Right eye fail <<<<<<<<<
        // 115, // Old guy, eyes very closed
        // 121
    };

    std::vector<std::string> ignoreImageList;
    transform(excludeList.begin(), excludeList.end(), std::back_inserter(ignoreImageList), [](const int i) {
        std::stringstream ss;
        ss << std::setw(3) << std::setfill('0') << i;
        return ss.str() + "_frontal.jpg";
    });

    std::vector<ResultData> resultsData;

    processDatabase(process,
                    ignoreImageList,
                    "research/mugshot_frontal_original_all/via_region_data_dpd.csv",
                    resultsData);

    processResults(resultsData);
}

TEST_F(PppEngineIntegrationTests, DevelopementTestSingleCase)
{
    auto imageFileName = resolvePath("research/mugshot_frontal_original_all/012_frontal.jpg");
    auto inputImage = imread(imageFileName);

    LandMarks detectedLandMarks;
    auto success = runLandMarkDetection(inputImage, detectedLandMarks);

    EXPECT_TRUE(success) << "Failed to process image " << imageFileName;

    using namespace cv;

    if (true)
    {
        Scalar detectionColor(250, 30, 0);
        rectangle(inputImage, detectedLandMarks.vjFaceRect, Scalar(0, 128, 0), 2);
        rectangle(inputImage, detectedLandMarks.vjLeftEyeRect, Scalar(0xA0, 0x52, 0x2D), 3);
        rectangle(inputImage, detectedLandMarks.vjRightEyeRect, Scalar(0xA0, 0x52, 0x2D), 3);

        polylines(inputImage,
                  std::vector<std::vector<Point>> { detectedLandMarks.lipContour1st, detectedLandMarks.lipContour2nd },
                  true,
                  detectionColor);
        rectangle(inputImage, detectedLandMarks.vjMouthRect, Scalar(0xA0, 0x52, 0x2D), 3);

        circle(inputImage, detectedLandMarks.eyeLeftPupil, 5, detectionColor, 2);
        circle(inputImage, detectedLandMarks.eyeRightPupil, 5, detectionColor, 2);

        circle(inputImage, detectedLandMarks.lipLeftCorner, 5, detectionColor, 2);
        circle(inputImage, detectedLandMarks.lipRightCorner, 5, detectionColor, 2);

        circle(inputImage, detectedLandMarks.crownPoint, 5, detectionColor, 2);
        circle(inputImage, detectedLandMarks.chinPoint, 5, detectionColor, 2);

        for (const auto & pt : detectedLandMarks.allLandmarks)
        {
            circle(inputImage, pt, 5, Scalar(40, 40, 190), 1);
        }
    }
}

#include <gtest/gtest.h>
#include <numeric>
#include <vector>

#include "IImageStore.h"
#include "LandMarks.h"
#include "PppEngine.h"
#include "TestHelpers.h"

#include <opencv2/imgcodecs/imgcodecs.hpp>

using namespace cv;

namespace ppp {
std::string pct(const double v)
{
    return std::to_string(v * 100) + "%";
}

class Stats final
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
        if (values.empty()) {
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

class LandMarkDetectionTests : public testing::Test
{
protected:
    PppEngineSPtr m_pPppEngine = std::make_shared<PppEngine>();

    void SetUp() override
    {
        const auto configFile = resolvePath("libppp/share/config.json");
        m_pPppEngine->configure(configFile);
    }

    static void processResults(const std::vector<ResultData> & resultsData)
    {
        std::vector<double> crownChinEstimationErrors;
        std::vector<double> scalingErrors;
        crownChinEstimationErrors.reserve(resultsData.size());
        scalingErrors.reserve(resultsData.size());
        for (const auto & r : resultsData) {
            EXPECT_TRUE(r.isSuccess);

            if (!r.isSuccess) {
                continue;
            }

            const auto errCrown = norm(r.annotation->crownPoint - r.detection->crownPoint);
            const auto errChin = norm(r.annotation->chinPoint - r.detection->chinPoint);
            const auto crownChinActualDist = norm(r.annotation->chinPoint - r.annotation->crownPoint);
            const auto crownChinEstimatedDist = norm(r.detection->chinPoint - r.detection->crownPoint);

            crownChinEstimationErrors.push_back((errCrown + errChin) / crownChinActualDist);
            scalingErrors.push_back(abs(crownChinEstimatedDist - crownChinActualDist) / crownChinActualDist);
        }

        const Stats s(scalingErrors);
        const Stats crownChinStats(crownChinEstimationErrors);
        std::cout << "Scale errors: " << s << std::endl;
        std::cout << "Crown-Chin estimation relative error: " << crownChinStats << std::endl;
    }

    void runSingleImage(const std::string & imageFilePath) const
    {
        const auto & imageStore = m_pPppEngine->getImageStore();
        const auto imgKey = imageStore->setImage(imageFilePath);
        auto inputImage = imageStore->getImage(imgKey);
        const auto success = m_pPppEngine->detectLandMarks(imgKey);
        const auto detectedLandMarks = imageStore->getLandMarks(imgKey);
        EXPECT_TRUE(success) << "Failed to process image " << imageFilePath;
        renderLandmarksOnImage(inputImage, detectedLandMarks);
        // cv::imwrite("/src/sample1.png", inputImage);
    }
};

TEST_F(LandMarkDetectionTests, EndToEndDetectioWorks)
{
    std::vector<double> leftEyeErrors;
    std::vector<double> rightEyeErrors;
    std::vector<double> leftLipsErrors;
    std::vector<double> rightLipsErrors;
    std::vector<double> relativeErrors;

    const auto process = [&](const std::string & imageFileName,
                             const LandMarksSPtr & annotations) -> std::tuple<bool, cv::Mat, LandMarksSPtr> {
        const auto imagePrefix = getFileName(imageFileName);
        std::cout << " Processing " << imagePrefix << std::endl;
        const auto & imageStore = m_pPppEngine->getImageStore();
        const auto imgKey = imageStore->setImage(imageFileName);
        const auto rgbImage = imageStore->getImage(imgKey);

        const auto success = m_pPppEngine->detectLandMarks(imgKey);
        const auto detectedLandMarks = imageStore->getLandMarks(imgKey);

        EXPECT_TRUE(success) << "Error detecting landmarks in " << imagePrefix;

        const double maxEyeAllowedError = 20;
        const auto leftEyeError = norm(detectedLandMarks->eyeLeftPupil - annotations->eyeLeftPupil);
        const auto rightEyeError = norm(detectedLandMarks->eyeRightPupil - annotations->eyeRightPupil);

        leftEyeErrors.push_back(leftEyeError);
        rightEyeErrors.push_back(rightEyeError);

        if (leftEyeError > maxEyeAllowedError) {
            std::cout << " Problem detecting left eye in " << imagePrefix << std::endl;
        }
        if (rightEyeError > maxEyeAllowedError) {
            std::cout << " Problem detecting right eye in " << imagePrefix << std::endl;
        }

        // Validate chin-crown distance error
        const auto expectedDistance = norm(annotations->chinPoint - annotations->crownPoint);
        const auto actualDistance = norm(detectedLandMarks->chinPoint - detectedLandMarks->crownPoint);

        constexpr auto MAX_ERROR = (36.0 - 32.0) / 34.0;
        const auto relError = abs(expectedDistance - actualDistance) / expectedDistance;
        const auto accepted = relError < MAX_ERROR;

        if (!accepted) {
            std::cout << " *** Estimation of face height too large for image " << imagePrefix << std::endl;
        }
        return { accepted, rgbImage, detectedLandMarks };
    };

    std::vector<int> excludeList = {
        74, // Eyes totally closed
    };

    std::vector<std::string> ignoreImageList;
    transform(excludeList.begin(), excludeList.end(), std::back_inserter(ignoreImageList), [](const int i) {
        std::stringstream ss;
        ss << std::setw(3) << std::setfill('0') << i << "_frontal.jpg";
        return ss.str();
    });

    auto resultsData = processDatabase(process,
                                       "research/mugshot_frontal_original_all/via_region_data_dpd.csv",
                                       {},
                                       ignoreImageList);

    processResults(resultsData);
}

TEST_F(LandMarkDetectionTests, DevelopmentTestSingleCase)
{
    runSingleImage(resolvePath("research/mugshot_frontal_original_all/077_frontal.jpg"));
}

TEST_F(LandMarkDetectionTests, DISABLED_babyTest)
{
    runSingleImage(resolvePath("research/my_database/20191021_155155.jpg"));
}

TEST_F(LandMarkDetectionTests, FujifilmImage)
{
    runSingleImage(resolvePath("research/my_database/DSCF0070.JPG"));
}
} // namespace ppp

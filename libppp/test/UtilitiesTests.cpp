#include "Utilities.h"

#include <gtest/gtest.h>
#include <utility>
#include "CommonHelpers.h"
#include "TestHelpers.h"
#include <opencv2/imgcodecs.hpp>

using namespace std;
using namespace cv;

TEST(UtilitiesTests, TestCalculatePointAtDistance)
{
    vector<tuple<Point2d, Point2d, double, Point2d, Point2d> > data =
    {
        make_tuple<Point2d, Point2d, double, Point2d, Point2d>(
        Point2d(1, 1), Point2d(3, 3), 1.0, Point2d(2 - sqrt(0.5), 2 + sqrt(0.5)), Point2d(2 + sqrt(0.5), 2 - sqrt(0.5))
        ),
        make_tuple<Point2d, Point2d, double, Point2d, Point2d>(
        Point2d(2, 1), Point2d(4, 1), 2.0, Point2d(3, -1), Point2d(3, 3)
        ),
        make_tuple<Point2d, Point2d, double, Point2d, Point2d>(
        Point2d(1,2), Point2d(1,4), 2.0, Point2d(-1, 3), Point2d(3, 3)
        ),
    };

    for (auto &t : data)
    {
        auto result = Utilities::pointsAtDistanceNormalToCentreOf(get<0>(t), get<1>(t), get<2>(t));
        EXPECT_EQ(get<3>(t), result.first);
        EXPECT_EQ(get<4>(t), result.second);
    }
}

TEST(UtilitiesTests, TestBase64EncodeDecode)
{
    auto testCases = {
        vector<uint8_t> {0},
        vector<uint8_t> {255},
        vector<uint8_t> {0, 1, 3, 43, 98, 34},
        vector<uint8_t> {128, 129, 254, 200},
        vector<uint8_t> {128, 129, 254, 200, 3, 0, 0, 89}
    };

    for (const auto &tc : testCases)
    {
        auto base64 = Utilities::base64Encode(tc);
        auto decode = Utilities::base64Decode(base64);
        EXPECT_TRUE(std::equal(tc.begin(), tc.end(), decode.begin()));
    }
}


TEST(UtilitiesTests, SelfCoefficientImageTests1)
{

    auto imageBear = resolvePath("research/mugshot_frontal_original_all/");

    auto inputImage = cv::imread(imageBear, cv::IMREAD_GRAYSCALE);

    cv::Mat outputImage = Utilities::selfCoefficientImage(inputImage, 7);

}
#include "Utilities.h"
//#include "EmbeddedContent.h"

#include "TestHelpers.h"
#include <gtest/gtest.h>
#include <opencv2/imgcodecs.hpp>
#include <utility>

using namespace std;
using namespace cv;

namespace ppp
{

TEST(UtilitiesTests, TestCalculatePointAtDistance)
{
    vector<tuple<Point2d, Point2d, double, Point2d, Point2d>> data = {
        make_tuple<Point2d, Point2d, double, Point2d, Point2d>(Point2d(1, 1),
                                                               Point2d(3, 3),
                                                               1.0,
                                                               Point2d(2 - sqrt(0.5), 2 + sqrt(0.5)),
                                                               Point2d(2 + sqrt(0.5), 2 - sqrt(0.5))),
        make_tuple<Point2d, Point2d, double, Point2d, Point2d>(Point2d(2, 1),
                                                               Point2d(4, 1),
                                                               2.0,
                                                               Point2d(3, -1),
                                                               Point2d(3, 3)),
        make_tuple<Point2d, Point2d, double, Point2d, Point2d>(Point2d(1, 2),
                                                               Point2d(1, 4),
                                                               2.0,
                                                               Point2d(-1, 3),
                                                               Point2d(3, 3)),
    };

    for (auto & t : data)
    {
        auto result = Utilities::pointsAtDistanceNormalToCentreOf(get<0>(t), get<1>(t), get<2>(t));
        EXPECT_EQ(get<3>(t), result.first);
        EXPECT_EQ(get<4>(t), result.second);
    }
}

TEST(UtilitiesTests, TestBase64EncodeDecode)
{
    const auto testCases = vector<vector<uint8_t>> { vector<uint8_t> { 0 },
                                                     vector<uint8_t> { 255 },
                                                     vector<uint8_t> { 0, 1, 3, 43, 98, 34 },
                                                     vector<uint8_t> { 128, 129, 254, 200 },
                                                     vector<uint8_t> { 128, 129, 254, 200, 3, 0, 0, 89 } };

    for (const auto & tc : testCases)
    {
        auto base64 = Utilities::base64Encode(tc);
        auto decode = Utilities::base64Decode(base64.c_str(), base64.size());
        EXPECT_TRUE(std::equal(tc.begin(), tc.end(), decode.begin()));
    }
}

// TEST(UtilitiesTests, ResourceRetrievalWorks)
//{
//    string fileName = "shape_predictor_68_face_landmarks.dat";
//
//    auto readMeFile = resolvePath(fileName);
//    auto fileContent = res::getFileContent(fileName);
//    std::string actual(fileContent.first, fileContent.first + fileContent.second);
//    std::ifstream t(readMeFile, std::ios::binary);
//    std::string expected((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());
//    ASSERT_EQ(actual, expected);
//}
} // namespace ppp

TEST(UtilititesTests, TestLineIntersection)
{

    EXPECT_THROW(Utilities::lineLineIntersection(Point2d(0, 0), Point2d(1, 1), Point2d(1, 0), Point2d(2, 1)),
                 std::runtime_error);

    // Valid test cases
    const auto testCases = vector<tuple<Point2d, Point2d, Point2d, Point2d, Point2d>> {
        make_tuple(Point2d(-1, -1), Point2d(1, 1), Point2d(-1, 1), Point2d(1, -1), Point2d(0, 0)),
        make_tuple(Point2d(15, 10), Point2d(49, 25), Point2d(29, 5), Point2d(22, 28), Point2d(26.0011, 14.8534))
    };

    for (const auto & tc : testCases)
    {
        const auto intersectPoint = Utilities::lineLineIntersection(get<0>(tc), get<1>(tc), get<2>(tc), get<3>(tc));
        const auto & expectedPoint = get<4>(tc);
        EXPECT_LE(norm(expectedPoint - intersectPoint), 0.01);
    }
}

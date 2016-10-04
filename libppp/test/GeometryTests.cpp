#include "Geometry.h"

#include <gtest/gtest.h>
#include <utility>

using namespace std;
using namespace cv;

TEST(GeometryTests, TestCalculatePointAtDistance)
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
        auto result = pointsAtDistanceNormalToCentreOf(get<0>(t), get<1>(t), get<2>(t));
        EXPECT_EQ(get<3>(t), result.first);
        EXPECT_EQ(get<4>(t), result.second);
    }
}

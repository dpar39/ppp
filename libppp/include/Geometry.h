#pragma once

#include <cmath>
#include <utility>
#include <opencv2/core/core.hpp>

#define ROUND_INT(x) (static_cast<int>((x)+0.5))

#define CEIL_INT(x) (static_cast<int>(ceil(x)))

#define POINT2D(p) (cv::Point2d((p).x, (p).y))

#define CENTER_POINT(p1, p2) (cv::Point2d((p1.x + p2.x)/2.0, (p1.y + p2.y)/2.0))

inline std::pair<cv::Point2d, cv::Point2d> pointsAtDistanceNormalToCentreOf(cv::Point2d p1, cv::Point2d p2, double d)
{
    if (p1 == p2)
    {
        throw std::runtime_error("Input points cannot be equal");
    }

    auto p0 = CENTER_POINT(p1, p2);
    cv::Point2d pa, pb; // Points at distance d from the normal line passing from the center of p1 and p2 (i.e. p0)
    if (p1.x == p2.x)
    {
        pa = pb = p0;
        pa.x -= d;
        pb.x += d;
    }
    else if (p1.y == p2.y)
    {
        pa = pb = p0;
        pa.y -= d;
        pb.y += d;
    }
    else
    {
        auto m = (p1.x - p2.x) / (p2.y - p1.y); // m' = -1/m
        auto dx = d / sqrt(1 + m*m);
        if (m < 0) dx = -dx;
        pa.x = p0.x + dx;
        pb.x = p0.x - dx;
        pa.y = m*(pa.x - p0.x) + p0.y;
        pb.y = m*(pb.x - p0.x) + p0.y;
    }
    return std::pair<cv::Point2d, cv::Point2d>(pa, pb);
}

/*!@brief Calculates the coordinate of a point in the line defined by two points
*  and that is at the specified distance of the first point towards the second point
*  @param[in] p0 First point of reference
*  @param[in] p1 Second point that define the line
*  @returns Calculated point
!*/
inline cv::Point2d pointInLineAtDistance(cv::Point2d p0, cv::Point2d p1, double dist)
{
    if (p1 == p0)
    {
        throw std::runtime_error("Input points cannot be equal");
    }
    auto ratio = dist / cv::norm(p1 - p0);
    return p0 + (p1 - p0)*ratio;
}

/*!@brief Calculates the intersection points between a line and a contour
*  @param[in] contour vector of 2D points
*  @param[in] pline1 First point defining the line
*  @param[in] pline2 Second point defining the line
*  @returns Vector of intersection point between the contour and the line
!*/
inline std::vector<cv::Point2d> contourLineIntersection(const std::vector<cv::Point> contour, cv::Point2d pline1, cv::Point2d pline2)
{
    std::vector<cv::Point2d> result;
    auto A2 = pline2.y - pline1.y;
    auto B2 = pline1.x - pline2.x;
    auto C2 = A2 * pline1.x + B2 * pline1.y;

    auto numVertex = contour.size();
    auto numSegments = contour.front() == contour.back() ? numVertex - 1 : numVertex;
    for (size_t i = 0; i < numSegments; i++)
    {
        cv::Point2d pSeg1 = contour[i];
        cv::Point2d pSeg2 = contour[(i + 1) % numVertex];
        auto A1 = pSeg2.y - pSeg1.y;
        auto B1 = pSeg1.x - pSeg2.x;
        auto C1 = A1 * pSeg1.x + B1 * pSeg1.y;
        auto det = A1 * B2 - A2 * B1;
        if (det != 0)
        {
            auto x = (B2 * C1 - B1 * C2) / det;
            auto y = (A1 * C2 - A2 * C1) / det;
            // Test if the intersection is in the segment
            auto x1 = pSeg1.x < pSeg2.x ? pSeg1.x : pSeg2.x;
            auto x2 = pSeg1.x > pSeg2.x ? pSeg1.x : pSeg2.x;
            auto y1 = pSeg1.y < pSeg2.y ? pSeg1.y : pSeg2.y;
            auto y2 = pSeg1.y > pSeg2.y ? pSeg1.y : pSeg2.y;
            if (x >= x1 && x <= x2 && y >= y1 && y <= y2)
                result.push_back(cv::Point2d(x, y));
        }
    }
    return result;
}

inline int kittlerOptimumThreshold(std::vector<double> P, float mu)
{
    int t, i, first = 0, last = 0, opt_threshold = 0, set = 0;
    double  q1[255], q2[255], mu1[255], mu2[255], var1[255], var2[255], H[255];
    printf("------------------------------\n");
    printf("\nPerforming the Optimization Operation\n");
    printf("TEST : %f \n", P[134]);


    printf(" --------------------------------------------------\n");
    printf("Done with Initialization of Gaussian mixture parameters\n");

    /* FINDING THE FIRST AND LAST OCCUPIED BINS */
    for (t = 1; t < 255; t++)
    {
        if (P[t] != 0 && P[t + 1] != 0 && first > t)
            first = t;
        if (P[t] != 0 && P[t + 1] != 0)
            last = t;
    }
    printf("FIRST %d LAST %d\n", first, last);

    /* INITIALIZATION OF PARAMETERS OF THE TWO GAUSSIAN MIXTURES */
    q1[first] = P[first];
    q2[first] = 1 - q1[first];
    mu1[first] = first;
    mu2[first] = (mu - mu1[first] * q1[first]) / q2[first];
    var1[first] = (first - mu1[first])*(first - mu1[first])*P[first] / q1[first];
    var2[first] = 0;
    for (i = first; i < last; i++)
    {
        var2[first] += (i - mu2[first])*(i - mu2[first])*P[i] / q2[first];
    }

    /* RUN THROUGH THE THRESHOLDS FOR OPTIMA */
    for (t = first + 1; t < last; t++)
    {
        q1[t] = q1[t - 1] + P[t];
        q2[t] = 1 - q1[t];
        mu1[t] = (q1[t - 1] * mu1[t - 1] + static_cast<double>(t)*P[t]) / q1[t];
        mu2[t] = (mu - q1[t] * mu1[t]) / q2[t];

        /* ENERGY FUNCTION */
        var1[t] = (q1[t - 1] * (var1[t - 1] + (mu1[t - 1] - mu1[t]) *(mu1[t - 1] - mu1[t])) + P[t] * (t - mu1[t])*(t - mu1[t])) / q1[t];
        var2[t] = (q2[t - 1] * (var2[t - 1] + (mu2[t - 1] - mu2[t]) *(mu2[t - 1] - mu2[t])) - P[t] * (t - mu2[t])*(t - mu2[t])) / q2[t];

        H[t] = (q1[t] * log(var1[t]) + q2[t] * log(var2[t])) / 2 - q1[t] * log(q1[t]) - q2[t] * log(q2[t]);
        printf("Energy %d: %lf\n", t, H[t]);
    }

    printf("\n\n------------------------------------------------------\n");
    printf(" Done with Iterative Procedure for Kittler's Algorithm\n\n");

    /* FIND OPTIMUM THRESHOLD (GLOBAL MINIMA)*/
    auto min = 9999.999;
    for (i = first + 1; i < last - 1; i++)
    {
        if (H[i] < min)
        {
            min = H[i];
            opt_threshold = i;
        }
    }
    printf(" \n\nThe minimum energy function is %lf and the threshold value is %d\n\n", min, opt_threshold);


    return opt_threshold;
}

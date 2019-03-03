#pragma once

#include <cmath>
#include <opencv2/core/core.hpp>
#include <utility>

#include "CommonHelpers.h"
#include <dlib/geometry/point_transforms.h>

namespace dlib
{
class rectangle;
typedef vector<long, 2> point;

} // namespace dlib

template <typename TNumber>
int ROUND_INT(TNumber x)
{
    return static_cast<int>((x) + 0.5);
}

template <typename TNumber>
int CEIL_INT(TNumber x)
{
    return static_cast<int>(ceil(x));
}

class Utilities
{
public:
    /*!@brief Loads a cascade classifier from file
    *  @param[in] haarCascadeBase64Data Haar cascade XML data encoded as a base64 string
    *  @returns The classifier loaded into memory
    !*/
    // static std::shared_ptr<cv::CascadeClassifier> loadClassifierFromFile(const std::string &haarCascadeDir, const
    // std::string &haarCascadeFile);
    static std::shared_ptr<cv::CascadeClassifier> loadClassifierFromBase64(const char * haarCascadeBase64Data);

    /*!@brief Calculates CRC value for a buffer of specified length !*/
    static uint32_t crc32(uint32_t crc, const uint8_t * begin, const uint8_t * end);

    static std::vector<BYTE> base64Decode(const char * base64Str, size_t base64Len);

    static std::string base64Encode(const std::vector<BYTE> & rawStr);

    /*!@brief Convert a distance to millimeters
    * @param[in] v Value to convert
    * @param[in] units Units (accepted values are "inch", "mm" and "cm"
    !*/
    static double toMM(double v, const std::string & units);

    static std::pair<cv::Point2d, cv::Point2d> pointsAtDistanceNormalToCentreOf(cv::Point2d p1,
                                                                                cv::Point2d p2,
                                                                                double d);

    /*!@brief Calculates the coordinate of a point in the line defined by two points
    *  and that is at the specified distance of the first point towards the second point
    *  @param[in] p0 First point of reference
    *  @param[in] p1 Second point that define the line
    *  @returns Calculated point
    !*/
    static cv::Point2d pointInLineAtDistance(cv::Point2d p0, cv::Point2d p1, double dist);

    /*!@brief Calculates the intersection points between a line and a contour
    *  @param[in] contour vector of 2D points
    *  @param[in] pline1 First point defining the line
    *  @param[in] pline2 Second point defining the line
    *  @returns Vector of intersection point between the contour and the line
    !*/
    static std::vector<cv::Point2d> contourLineIntersection(const std::vector<cv::Point> contour,
                                                            cv::Point2d pline1,
                                                            cv::Point2d pline2);

    /**
     * \brief Not being used so far
     * \param P
     * \param mu
     * \return
     */
    static int kittlerOptimumThreshold(std::vector<double> P, float mu);

    static cv::Mat selfCoefficientImage(const cv::Mat & inputImage, int kernelSize);

    static cv::Point convert(const dlib::point & pt);

    static cv::Rect2d convert(const dlib::rectangle & r);

    static dlib::rectangle convert(const cv::Rect2d & r);
};

#pragma once

#include <cmath>
#include <opencv2/core/core.hpp>

#include "CommonHelpers.h"
#include <dlib/geometry/point_transforms.h>
#include <rapidjson/document.h>
#include <rapidjson/rapidjson.h>

namespace dlib
{
class rectangle;
typedef vector<long, 2> point;

} // namespace dlib

template <typename TNumber>
int roundInteger(TNumber x)
{
    return static_cast<int>((x) + 0.5);
}

template <typename TNumber>
int ceilInteger(TNumber x)
{
    return static_cast<int>(ceil(x));
}

template <typename TNumber>
int floorInteger(TNumber x)
{
    return static_cast<int>(floor(x));
}

class Utilities final
{
public:
    /*!@brief Loads a cascade classifier from file
    *  @param[in] haarCascadeData Haar cascade XML data possibly encoded as a base64 string
    *  @returns The classifier loaded into memory
    !*/
    static std::shared_ptr<cv::CascadeClassifier> loadClassifierFromBase64(const char * haarCascadeData);

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
    *  @param[in] p1Line First point defining the line
    *  @param[in] p2Line Second point defining the line
    *  @returns Vector of intersection point between the contour and the line
    !*/
    static std::vector<cv::Point2d> contourLineIntersection(const std::vector<cv::Point> & contour,
                                                            cv::Point2d p1Line,
                                                            cv::Point2d p2Line);

    static cv::Mat rotateImage(const cv::Mat & inputImage, int rotationAngleDegrees);

    static cv::Point convert(const dlib::point & pt);

    static cv::Rect2d convert(const dlib::rectangle & r);

    /**
     * \brief Serializes a JSON document to std::string
     * \param d Json document to serialize
     * \return the serialized string for the Json document
     */
    static std::string serializeJson(rapidjson::Document & d, bool pretty);

    static void setPngResolutionDpi(std::vector<BYTE> & imageStream, double resolution_ppmm);

    static std::string encodeImageAsPng(const cv::Mat & image, bool encodeBase64, double resolution_ppmm = 0);

    /**
     * \brief Converts the value held by a variable into a byte vector in Little Endian notation
     * \tparam T Type of the variable to be serialized  to bytes
     * \param x Variable being serialized to a byte array
     * \return A vector containing the bytes in Little Endian notation
     */
    template <typename T>
    static std::vector<BYTE> toBytes(const T & x)
    {
        std::vector<BYTE> v(static_cast<const BYTE *>(static_cast<const void *>(&x)),
                            static_cast<const BYTE *>(static_cast<const void *>(&x)) + sizeof(x));
        std::reverse(v.begin(), v.end()); // Little Endian notation
        return v;
    }
};

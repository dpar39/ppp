#pragma once

#include "CommonHelpers.h"
#include <opencv2/core/core.hpp>

#include <rapidjson/document.h>
#include <rapidjson/rapidjson.h>

#include <cmath>
#include <concepts>

#define VALIDATE_GT(v, min)                                                                                            \
    if ((v) <= (min))                                                                                                  \
        throw std::runtime_error(std::string(#v) + " should be greater than " + std::to_string((min)));

#define VALIDATE_LT(v, max)                                                                                            \
    if ((v) >= (max))                                                                                                  \
        throw std::runtime_error(std::string(#v) + " should be less than " + std::to_string((max)));

#define VALIDATE_GE(v, min)                                                                                            \
    if ((v) < (min))                                                                                                   \
        throw std::runtime_error(std::string(#v) + " should be greater or equal than " + std::to_string((min)));

#define VALIDATE_LE(v, max)                                                                                            \
    if ((v) > (max))                                                                                                   \
        throw std::runtime_error(std::string(#v) + " should be less or equal than " + std::to_string((max)));

template <class T>
concept numeric = std::is_integral_v<T> || std::is_floating_point_v<T>;

int roundInteger(numeric auto x)
{
    return static_cast<int>(round(x));
}

int ceilInteger(numeric auto x)
{
    return static_cast<int>(ceil(x));
}

int floorInteger(numeric auto x)
{
    return static_cast<int>(floor(x));
}

class Utilities final
{
public:
    static std::vector<BYTE> base64Decode(const char * base64Str, size_t base64Len);

    static std::string base64Encode(const std::vector<BYTE> & rawStr);

    /**
     * \brief Converts a dimension to pixels
     * \param v Value to convert to pixels
     * \param units Value units (e.g. 'inch' or 'cm' or 'mm')
     * \param dpi Resolution in dots per inch
     * \return Equivalent value in pixels
     */
    static double toPixels(double v, const std::string & units, double dpi);

    static double fromPixel(double v, const std::string & units, double dpi);

    static double convert(double v, const std::string & from, const std::string & to, double dpi);

    static std::pair<cv::Point2d, cv::Point2d> pointsAtDistanceNormalToCentreOf(const cv::Point2d & p1,
                                                                                const cv::Point2d & p2,
                                                                                double d);

    /*!@brief Calculates the coordinate of a point in the line defined by two points
    *  and that is at the specified distance of the first point towards the second point
    *  @param[in] p0 First point of reference
    *  @param[in] p1 Second point that define the line
    *  @returns Calculated point
    !*/
    static cv::Point2d pointInLineAtDistance(const cv::Point2d & p0, const cv::Point2d & p1, double dist);

    /*!@brief Calculates the intersection points between a line and a contour
    *  @param[in] contour vector of 2D points
    *  @param[in] p1Line First point defining the line
    *  @param[in] pLine2 Second point defining the line
    *  @returns Vector of intersection point between the contour and the line
    !*/
    static std::vector<cv::Point2d> contourLineIntersection(const std::vector<cv::Point> & contour,
                                                            const cv::Point2d & p1Line,
                                                            const cv::Point2d & pLine2);
    static cv::Point2d lineLineIntersection(const cv::Point2d & p1,
                                            const cv::Point2d & p2,
                                            const cv::Point2d & q1,
                                            const cv::Point2d & q2);

    /**
     * \brief Serializes a JSON document to std::string
     * \param d Json document to serialize
     * \param pretty If true, generated json is prettified, otherwise minified
     * \return the serialized string for the Json document
     */
    static std::string serializeJson(rapidjson::Document & d, bool pretty);

    static uint32_t computeCrc32(const uint8_t * start, const uint8_t * end);

    /**
     * \brief Sets or add the DPI resolution to PNG image
     * \param imageStream An array of bytes representing a PNG image
     * \param resolution_dpi Resolution to be set in the image (DPI)
     * \return true if the pHYs section was added, false if it was just replaced
     */
    static bool setPngResolutionDpi(std::vector<BYTE> & imageStream, double resolution_dpi);

    static std::string encodeImageAsPng(const cv::Mat & image, bool encodeBase64, double resolution_dpi = 0);

    static bool isBigEndian()
    {
        int16_t word = 0x0001;
        char * b = reinterpret_cast<char *>(&word); // NOLINT
        return b[0] == 0;
    }

    /**
     * \brief Converts the value held by a variable into a byte vector in Little Endian notation
     * \tparam T Type of the variable to be serialized  to bytes
     * \param x Variable being serialized to a byte array
     * \return A vector containing the bytes in Little Endian notation
     */
    template <typename T>
    static std::vector<BYTE> toBytes(const T & x, bool bigEndian = true)
    {
        std::vector<BYTE> v(static_cast<const BYTE *>(static_cast<const void *>(&x)),
                            static_cast<const BYTE *>(static_cast<const void *>(&x)) + sizeof(x));
        if ((isBigEndian() && !bigEndian) || (!isBigEndian() && bigEndian)) {
            std::reverse(v.begin(), v.end()); // Little Endian notation
        }
        return v;
    }

    template <class T>
    struct dependent_false : std::false_type
    {
    };
    template <typename T>
    static T getField(const rapidjson::Value & v, const std::string & fieldName, const T defaultValue)
    {
        if (!v.HasMember(fieldName)) {
            return defaultValue;
        }
        if constexpr (std::is_floating_point<T>::value) {
            return v[fieldName].GetDouble();
        } else if constexpr (std::is_integral<T>::value) {
            return v[fieldName].GetInt();
        } else if constexpr (std::is_same<bool, T>::value) {
            return v[fieldName].GetBool();
        } else if constexpr (std::is_same<std::string, T>::value || std::is_same<char *, T>::value) {
            return v[fieldName].GetString();
        } else {
            static_assert(dependent_false<T>::value, "Must be floating point, integral, boolean or string");
        }
        return defaultValue;
    }
};

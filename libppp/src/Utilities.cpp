#include "Utilities.h"

#include <numeric>

#include <dlib/geometry/rectangle.h>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/objdetect/objdetect.hpp>
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <unordered_set>

namespace cv
{
FWD_DECL(CascadeClassifier)
}

static uint8_t fromChar(const char ch)
{
    if (ch >= 'A' && ch <= 'Z')
    {
        return ch - 'A';
    }
    if (ch >= 'a' && ch <= 'z')
    {
        return 26 + (ch - 'a');
    }
    if (ch >= '0' && ch <= '9')
    {
        return 52 + (ch - '0');
    }
    if (ch == '+')
    {
        return 62;
    }
    if (ch == '/')
    {
        return 63;
    }
    throw std::runtime_error("Invalid character in base64 string");
}

std::vector<BYTE> Utilities::base64Decode(const char * base64Str, const size_t base64Len)
{
    BYTE charBlock4[4], byteBlock3[3];
    std::vector<BYTE> result;
    result.reserve(base64Len * 3 / 4);
    auto i = 0;
    for (size_t k = 0; k < base64Len; ++k)
    {
        const auto ch64 = base64Str[k];
        if (ch64 == '=')
        {
            break;
        }
        charBlock4[i++] = ch64;
        if (i == 4)
        {
            for (i = 0; i < 4; i++)
            {
                charBlock4[i] = fromChar(charBlock4[i]);
            }
            byteBlock3[0] = (charBlock4[0] << 2) + ((charBlock4[1] & 0x30) >> 4);
            byteBlock3[1] = ((charBlock4[1] & 0xf) << 4) + ((charBlock4[2] & 0x3c) >> 2);
            byteBlock3[2] = ((charBlock4[2] & 0x3) << 6) + charBlock4[3];
            result.insert(result.end(), byteBlock3, byteBlock3 + 3);
            i = 0;
        }
    }

    if (i > 0)
    {
        std::fill(charBlock4 + i, charBlock4 + 4, '\0');
        std::transform(charBlock4, charBlock4 + i, charBlock4, fromChar);

        byteBlock3[0] = (charBlock4[0] << 2) + ((charBlock4[1] & 0x30) >> 4);
        byteBlock3[1] = ((charBlock4[1] & 0xf) << 4) + ((charBlock4[2] & 0x3c) >> 2);
        byteBlock3[2] = ((charBlock4[2] & 0x3) << 6) + charBlock4[3];
        result.insert(result.end(), byteBlock3, byteBlock3 + i - 1);
    }
    return result;
}

struct Membuf : std::streambuf
{
    Membuf(char const * base, const size_t size)
    {
        char * p(const_cast<char *>(base));
        this->setg(p, p, p + size);
    }
};
struct Imemstream final : virtual Membuf, std::istream
{
    Imemstream(char const * base, const size_t size)
    : Membuf(base, size)
    , std::istream(static_cast<std::streambuf *>(this))
    {
    }
};

std::string Utilities::base64Encode(const std::vector<BYTE> & rawStr)
{

    auto byteIter = rawStr.data();
    auto bufferSize = rawStr.size();
    const std::string Base64CharSet = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    std::string result;
    result.reserve(static_cast<size_t>(ceil(bufferSize * 4 / 3.0)));
    auto i = 0;
    int j;
    uint8_t charArray3[3];
    uint8_t charArray4[4];

    while (bufferSize--)
    {
        charArray3[i++] = *byteIter++;
        if (i == 3)
        {
            charArray4[0] = (charArray3[0] & 0xfc) >> 2;
            charArray4[1] = ((charArray3[0] & 0x03) << 4) + ((charArray3[1] & 0xf0) >> 4);
            charArray4[2] = ((charArray3[1] & 0x0f) << 2) + ((charArray3[2] & 0xc0) >> 6);
            charArray4[3] = charArray3[2] & 0x3f;
            for (i = 0; i < 4; i++)
            {
                const auto idx = charArray4[i];
                result.push_back(Base64CharSet[idx]);
            }
            i = 0;
        }
    }

    if (i > 0)
    {
        for (j = i; j < 3; j++)
        {
            charArray3[j] = 0;
        }
        charArray4[0] = (charArray3[0] & 0xfc) >> 2;
        charArray4[1] = ((charArray3[0] & 0x03) << 4) + ((charArray3[1] & 0xf0) >> 4);
        charArray4[2] = ((charArray3[1] & 0x0f) << 2) + ((charArray3[2] & 0xc0) >> 6);
        charArray4[3] = charArray3[2] & 0x3f;
        for (j = 0; j < i + 1; j++)
        {
            const auto idx = charArray4[j];
            result.push_back(Base64CharSet[idx]);
        }
        while (i++ < 3)
        {
            result.push_back('=');
        }
    }
    return result;
}

cv::CascadeClassifierSPtr Utilities::loadClassifierFromBase64(const char * haarCascadeData)
{
    static const std::string XML_START = "<?xml";
    std::string xmlHaarCascadeStr;
    if (std::equal(XML_START.begin(), XML_START.end(), haarCascadeData))
    {
        xmlHaarCascadeStr.assign(haarCascadeData);
    }
    else
    {
        auto a = base64Decode(haarCascadeData, strlen(haarCascadeData));
        xmlHaarCascadeStr.assign(a.begin(), a.end());
    }
    auto classifier = std::make_shared<cv::CascadeClassifier>();
    try
    {
        cv::FileStorage fs(xmlHaarCascadeStr, cv::FileStorage::READ | cv::FileStorage::MEMORY);
        classifier->read(fs.getFirstTopLevelNode());
        if (classifier->empty())
        {
            throw std::runtime_error("Failed to load classifier from configuration");
        }
        return classifier;
    }
    catch (std::exception & e)
    {
        throw e;
    }
}

uint32_t Utilities::crc32(uint32_t crc, const uint8_t * begin, const uint8_t * end)
{
    /* Table of CRCs of all 8-bit messages. */
    static uint32_t s_crcTable[256];
    static std::once_flag s_crcComputeFlag;
    std::call_once(s_crcComputeFlag, []() {
        for (auto n = 0; n < 256; n++)
        {
            auto c = static_cast<unsigned long>(n);
            for (auto k = 0; k < 8; k++)
            {
                if (c & 1)
                {
                    c = 0xedb88320L ^ (c >> 1);
                }
                else
                {
                    c = c >> 1;
                }
            }
            s_crcTable[n] = c;
        }
    });

    auto it = begin;
    while (it != end)
    {
        crc = s_crcTable[(crc ^ *it++) & 0xff] ^ (crc >> 8);
    }
    return crc;
}

cv::Mat Utilities::rotateImage(const cv::Mat & inputImage, const int rotationAngleDegrees)
{
    if (rotationAngleDegrees == 0)
        return inputImage;

    static const std::set<int> validAngles = { 90, 180, 270 };
    const auto absAngle = std::abs(rotationAngleDegrees);
    if (validAngles.find(absAngle) == validAngles.end())
        throw std::logic_error("Provided rotation angle is not supported.");

    const auto h = inputImage.size().height;
    const auto w = inputImage.size().width;

    const auto M = cv::getRotationMatrix2D(cv::Point2f(h / 2.0, w / 2.0), rotationAngleDegrees, 1);
    const auto size = absAngle == 90 || absAngle == 270 ? cv::Size(h, w) : cv::Size(w, h);
    cv::Mat rotatedImage;
    cv::warpAffine(inputImage, rotatedImage, M, size);
    return rotatedImage;
}

/* Update a running CRC with the bytes buf[0..len-1]--the CRC
should be initialized to all 1's, and the transmitted value
is the 1's complement of the final running CRC (see the
crc() routine below)). */
uint32_t updateCrc(uint32_t crc, const unsigned char * buf, const size_t len)
{
    /* Table of CRCs of all 8-bit messages. */
    static uint32_t s_crcTable[256];
    static std::once_flag s_crcComputeFlag;
    std::call_once(s_crcComputeFlag, []() {
        for (auto n = 0; n < 256; n++)
        {
            auto c = static_cast<unsigned long>(n);
            for (auto k = 0; k < 8; k++)
            {
                if (c & 1)
                {
                    c = 0xedb88320L ^ (c >> 1);
                }
                else
                {
                    c = c >> 1;
                }
            }
            s_crcTable[n] = c;
        }
    });
    for (size_t n = 0; n < len; n++)
    {
        crc = s_crcTable[(crc ^ buf[n]) & 0xff] ^ (crc >> 8);
    }
    return crc;
}

double Utilities::toPixels(const double v, const std::string & units, const double dpi)
{
    if (units == "pixel")
    {
        return v;
    }
    if (units == "inch")
    {
        return v * dpi;
    }
    if (units == "mm")
    {
        return v * dpi / 25.4;
    }
    if (units == "cm")
    {
        return v * dpi / 2.54;
    }
    throw std::runtime_error("Unknown input units when converting to pixels");
}

double Utilities::fromPixel(const double v, const std::string & units, const double dpi)
{
    if (units == "pixel")
    {
        return v;
    }
    if (units == "inch")
    {
        return v / dpi;
    }
    if (units == "mm")
    {
        return v / dpi * 25.4;
    }
    if (units == "cm")
    {
        return v / dpi * 2.54;
    }
    throw std::runtime_error("Unknown input units when converting to pixels");
}

double Utilities::convert(const double v, const std::string & from, const std::string & to, const double dpi)
{
    if (from == to)
    {
        return v; // Nothing to convert, great!
    }
    const auto pixEquivalent = toPixels(v, from, dpi);
    return fromPixel(pixEquivalent, to, dpi);
}

std::pair<cv::Point2d, cv::Point2d> Utilities::pointsAtDistanceNormalToCentreOf(const cv::Point2d & p1,
                                                                                const cv::Point2d & p2,
                                                                                const double d)
{
    if (p1 == p2)
    {
        throw std::runtime_error("Input points cannot be equal");
    }

    const auto p0 = (p1 + p2) / 2.0;
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
        const auto m = (p1.x - p2.x) / (p2.y - p1.y); // m' = -1/m
        auto dx = d / sqrt(1 + m * m);
        if (m < 0)
            dx = -dx;
        pa.x = p0.x + dx;
        pb.x = p0.x - dx;
        pa.y = m * (pa.x - p0.x) + p0.y;
        pb.y = m * (pb.x - p0.x) + p0.y;
    }
    return std::pair<cv::Point2d, cv::Point2d>(pa, pb);
}

cv::Point2d Utilities::pointInLineAtDistance(cv::Point2d p0, cv::Point2d p1, double dist)
{
    if (p1 == p0)
    {
        throw std::runtime_error("Input points cannot be equal");
    }
    const auto ratio = dist / norm(p1 - p0);
    return p0 + (p1 - p0) * ratio;
}

std::vector<cv::Point2d> Utilities::contourLineIntersection(const std::vector<cv::Point> & contour,
                                                            const cv::Point2d pLine1,
                                                            const cv::Point2d pLine2)
{
    std::vector<cv::Point2d> result;
    const auto A2 = pLine2.y - pLine1.y;
    const auto B2 = pLine1.x - pLine2.x;
    const auto C2 = A2 * pLine1.x + B2 * pLine1.y;

    const auto numVertex = contour.size();
    const auto numSegments = contour.front() == contour.back() ? numVertex - 1 : numVertex;
    for (size_t i = 0; i < numSegments; i++)
    {
        cv::Point2d pSeg1 = contour[i];
        cv::Point2d pSeg2 = contour[(i + 1) % numVertex];
        const auto A1 = pSeg2.y - pSeg1.y;
        const auto B1 = pSeg1.x - pSeg2.x;
        const auto C1 = A1 * pSeg1.x + B1 * pSeg1.y;
        const auto det = A1 * B2 - A2 * B1;
        if (det != 0)
        {
            const auto x = (B2 * C1 - B1 * C2) / det;
            const auto y = (A1 * C2 - A2 * C1) / det;
            // Test if the intersection is in the segment
            const auto x1 = pSeg1.x < pSeg2.x ? pSeg1.x : pSeg2.x;
            const auto x2 = pSeg1.x > pSeg2.x ? pSeg1.x : pSeg2.x;
            const auto y1 = pSeg1.y < pSeg2.y ? pSeg1.y : pSeg2.y;
            const auto y2 = pSeg1.y > pSeg2.y ? pSeg1.y : pSeg2.y;
            if (x >= x1 && x <= x2 && y >= y1 && y <= y2)
                result.emplace_back(x, y);
        }
    }
    return result;
}

cv::Point2d Utilities::lineLineIntersection(const cv::Point2d & p1,
                                            const cv::Point2d & p2,
                                            const cv::Point2d & q1,
                                            const cv::Point2d & q2)
{
    const auto x1 = p1.x;
    const auto y1 = p1.y;
    const auto x2 = p2.x;
    const auto y2 = p2.y;

    const auto x3 = q1.x;
    const auto y3 = q1.y;
    const auto x4 = q2.x;
    const auto y4 = q2.y;

    const auto d = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4);
    if (d == 0)
    {
        throw std::runtime_error("Lines are parallel ");
    }

    const auto a = (x1 * y2 - y1 * x2);
    const auto b = (x3 * y4 - y3 * x4);
    const auto x = (a * (x3 - x4) - b * (x1 - x2)) / d;
    const auto y = (a * (y3 - y4) - b * (y1 - y2)) / d;
    return cv::Point2d(x, y);
}

cv::Point Utilities::convert(const dlib::point & pt)
{
    return cv::Point2d(pt.x(), pt.y());
}

cv::Rect2d Utilities::convert(const dlib::rectangle & r)
{
    return cv::Rect2d(r.left(), r.top(), r.width(), r.height());
}

/*  The pHYs chunk specifies the intended pixel size or aspect ratio for display of the image. It contains:
    Pixels per unit, X axis: 4 bytes (unsigned integer)
    Pixels per unit, Y axis: 4 bytes (unsigned integer)
    Unit specifier:          1 byte
The following values are defined for the unit specifier:
    0: unit is unknown
    1: unit is the meter
pHYs has to go before IDAT chunk
*/

void Utilities::setPngResolutionDpi(std::vector<BYTE> & imageStream, const double resolution_dpi)
{
    const auto chunkLenBytes = toBytes(9);
    auto resolutionBytes = toBytes(roundInteger(resolution_dpi * 1000.0 / 25.4));
    const std::string physStr = "pHYs";

    auto pHYsChunk(chunkLenBytes);
    pHYsChunk.insert(pHYsChunk.end(), physStr.begin(), physStr.end());

    pHYsChunk.insert(pHYsChunk.end(), resolutionBytes.begin(), resolutionBytes.end());
    pHYsChunk.insert(pHYsChunk.end(), resolutionBytes.begin(), resolutionBytes.end());
    pHYsChunk.push_back(1); // Unit is the meter

    auto crcBytes = toBytes(crc32(0, &pHYsChunk[4], &pHYsChunk[4] + pHYsChunk.size() - 4));
    pHYsChunk.insert(pHYsChunk.end(), crcBytes.begin(), crcBytes.end());

    static const std::string idat = "IDAT";
    const auto it = search(imageStream.begin(), imageStream.end(), idat.begin(), idat.end());
    if (it != imageStream.end())
    {
        // Insert the chunk in the stream
        imageStream.insert(it - 4, pHYsChunk.begin(), pHYsChunk.end());
    }
}

std::string Utilities::encodeImageAsPng(const cv::Mat & image, const bool encodeBase64, double resolution_dpi)
{
    std::vector<BYTE> pictureData;
    imencode(".png", image, pictureData);
    if (resolution_dpi > 0)
    {
        setPngResolutionDpi(pictureData, resolution_dpi);
    }
    if (encodeBase64)
    {
        return base64Encode(pictureData);
    }
    return std::string(pictureData.begin(), pictureData.end());
}

std::string Utilities::serializeJson(rapidjson::Document & d, const bool pretty)
{
    rapidjson::StringBuffer buffer;
    if (pretty)
    {
        rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
        d.Accept(writer);
    }
    else
    {
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        d.Accept(writer);
    }
    return std::string(buffer.GetString());
}

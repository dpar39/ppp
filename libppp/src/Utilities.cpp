#include "Utilities.h"

#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/objdetect/objdetect.hpp>
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include <array>
#include <numeric>

namespace cv {
FWD_DECL(CascadeClassifier)
}

static uint8_t fromChar(const char ch)
{
    if (ch >= 'A' && ch <= 'Z') {
        return ch - 'A';
    }
    if (ch >= 'a' && ch <= 'z') {
        return 26 + (ch - 'a');
    }
    if (ch >= '0' && ch <= '9') {
        return 52 + (ch - '0');
    }
    if (ch == '+') {
        return 62;
    }
    if (ch == '/') {
        return 63;
    }
    throw std::runtime_error("Invalid character in base64 string");
}

std::vector<BYTE> Utilities::base64Decode(const char * base64Str, const size_t base64Len)
{
    BYTE charBlock4[4];
    BYTE byteBlock3[3];
    std::vector<BYTE> result;
    result.reserve(base64Len * 3 / 4);
    auto i = 0;
    for (size_t k = 0; k < base64Len; ++k) {
        const auto ch64 = base64Str[k];
        if (ch64 == '=') {
            break;
        }
        charBlock4[i++] = ch64;
        if (i == 4) {
            for (i = 0; i < 4; i++) {
                charBlock4[i] = fromChar(charBlock4[i]);
            }
            byteBlock3[0] = (charBlock4[0] << 2) + ((charBlock4[1] & 0x30) >> 4);
            byteBlock3[1] = ((charBlock4[1] & 0xf) << 4) + ((charBlock4[2] & 0x3c) >> 2);
            byteBlock3[2] = ((charBlock4[2] & 0x3) << 6) + charBlock4[3];
            result.insert(result.end(), byteBlock3, byteBlock3 + 3);
            i = 0;
        }
    }

    if (i > 0) {
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

    const auto * byteIter = rawStr.data();
    auto bufferSize = rawStr.size();
    const std::string base64CharSet = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    std::string result;
    result.reserve(static_cast<size_t>(ceil(bufferSize * 4 / 3.0)));
    auto i = 0;
    int j;
    std::array<uint8_t, 3> charArray3 {};
    std::array<uint8_t, 4> charArray4 {};

    while (bufferSize--) {
        charArray3[i++] = *byteIter++;
        if (i == 3) {
            charArray4[0] = (charArray3[0] & 0xfc) >> 2;
            charArray4[1] = ((charArray3[0] & 0x03) << 4) + ((charArray3[1] & 0xf0) >> 4);
            charArray4[2] = ((charArray3[1] & 0x0f) << 2) + ((charArray3[2] & 0xc0) >> 6);
            charArray4[3] = charArray3[2] & 0x3f;
            for (i = 0; i < 4; i++) {
                const auto idx = charArray4[i];
                result.push_back(base64CharSet[idx]);
            }
            i = 0;
        }
    }

    if (i > 0) {
        for (j = i; j < 3; j++) {
            charArray3[j] = 0;
        }
        charArray4[0] = (charArray3[0] & 0xfc) >> 2;
        charArray4[1] = ((charArray3[0] & 0x03) << 4) + ((charArray3[1] & 0xf0) >> 4);
        charArray4[2] = ((charArray3[1] & 0x0f) << 2) + ((charArray3[2] & 0xc0) >> 6);
        charArray4[3] = charArray3[2] & 0x3f;
        for (j = 0; j < i + 1; j++) {
            const auto idx = charArray4[j];
            result.push_back(base64CharSet[idx]);
        }
        while (i++ < 3) {
            result.push_back('=');
        }
    }
    return result;
}

double Utilities::toPixels(const double v, const std::string & units, const double dpi)
{
    if (units == "pixel") {
        return v;
    }
    if (units == "inch") {
        return v * dpi;
    }
    if (units == "mm") {
        return v * dpi / 25.4;
    }
    if (units == "cm") {
        return v * dpi / 2.54;
    }
    throw std::runtime_error("Unknown input units when converting to pixels");
}

double Utilities::fromPixel(const double v, const std::string & units, const double dpi)
{
    if (units == "pixel") {
        return v;
    }
    if (units == "inch") {
        return v / dpi;
    }
    if (units == "mm") {
        return v / dpi * 25.4;
    }
    if (units == "cm") {
        return v / dpi * 2.54;
    }
    throw std::runtime_error("Unknown input units when converting to pixels");
}

double Utilities::convert(const double v, const std::string & from, const std::string & to, const double dpi)
{
    if (from == to) {
        return v; // Nothing to convert, great!
    }
    const auto pixEquivalent = toPixels(v, from, dpi);
    return fromPixel(pixEquivalent, to, dpi);
}

std::pair<cv::Point2d, cv::Point2d> Utilities::pointsAtDistanceNormalToCentreOf(const cv::Point2d & p1,
                                                                                const cv::Point2d & p2,
                                                                                const double d)
{
    if (p1 == p2) {
        throw std::runtime_error("Input points cannot be equal");
    }

    const auto p0 = (p1 + p2) / 2.0;
    cv::Point2d pa;
    cv::Point2d pb; // Points at distance d from the normal line passing from the center of p1 and p2 (i.e. p0)
    if (p1.x == p2.x) {
        pa = pb = p0;
        pa.x -= d;
        pb.x += d;
    } else if (p1.y == p2.y) {
        pa = pb = p0;
        pa.y -= d;
        pb.y += d;
    } else {
        const auto m = (p1.x - p2.x) / (p2.y - p1.y); // m' = -1/m
        auto dx = d / sqrt(1 + m * m);
        if (m < 0) {
            dx = -dx;
        }
        pa.x = p0.x + dx;
        pb.x = p0.x - dx;
        pa.y = m * (pa.x - p0.x) + p0.y;
        pb.y = m * (pb.x - p0.x) + p0.y;
    }
    return std::pair<cv::Point2d, cv::Point2d>(pa, pb);
}

cv::Point2d Utilities::pointInLineAtDistance(const cv::Point2d & p0, const cv::Point2d & p1, double dist)
{
    if (p1 == p0) {
        throw std::runtime_error("Input points cannot be equal");
    }
    const auto ratio = dist / norm(p1 - p0);
    return p0 + (p1 - p0) * ratio;
}

std::vector<cv::Point2d> Utilities::contourLineIntersection(const std::vector<cv::Point> & contour,
                                                            const cv::Point2d & pLine1,
                                                            const cv::Point2d & pLine2)
{
    std::vector<cv::Point2d> result;
    const auto a2 = pLine2.y - pLine1.y;
    const auto b2 = pLine1.x - pLine2.x;
    const auto c2 = a2 * pLine1.x + b2 * pLine1.y;

    const auto numVertex = contour.size();
    const auto numSegments = contour.front() == contour.back() ? numVertex - 1 : numVertex;
    for (size_t i = 0; i < numSegments; i++) {
        cv::Point2d pSeg1 = contour[i];
        cv::Point2d pSeg2 = contour[(i + 1) % numVertex];
        const auto a1 = pSeg2.y - pSeg1.y;
        const auto b1 = pSeg1.x - pSeg2.x;
        const auto c1 = a1 * pSeg1.x + b1 * pSeg1.y;
        const auto det = a1 * b2 - a2 * b1;
        if (det != 0) {
            const auto x = (b2 * c1 - b1 * c2) / det;
            const auto y = (a1 * c2 - a2 * c1) / det;
            // Test if the intersection is in the segment
            const auto x1 = pSeg1.x < pSeg2.x ? pSeg1.x : pSeg2.x;
            const auto x2 = pSeg1.x > pSeg2.x ? pSeg1.x : pSeg2.x;
            const auto y1 = pSeg1.y < pSeg2.y ? pSeg1.y : pSeg2.y;
            const auto y2 = pSeg1.y > pSeg2.y ? pSeg1.y : pSeg2.y;
            if (x >= x1 && x <= x2 && y >= y1 && y <= y2) {
                result.emplace_back(x, y);
            }
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
    if (d == 0) {
        throw std::runtime_error("Lines are parallel ");
    }

    const auto a = (x1 * y2 - y1 * x2);
    const auto b = (x3 * y4 - y3 * x4);
    const auto x = (a * (x3 - x4) - b * (x1 - x2)) / d;
    const auto y = (a * (y3 - y4) - b * (y1 - y2)) / d;
    return { x, y };
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

static uint32_t crc_32_tab[]
    = { /* CRC polynomial 0xedb88320 */
        0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f, 0xe963a535, 0x9e6495a3, 0x0edb8832,
        0x79dcb8a4, 0xe0d5e91e, 0x97d2d988, 0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91, 0x1db71064, 0x6ab020f2,
        0xf3b97148, 0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7, 0x136c9856, 0x646ba8c0, 0xfd62f97a,
        0x8a65c9ec, 0x14015c4f, 0x63066cd9, 0xfa0f3d63, 0x8d080df5, 0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
        0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b, 0x35b5a8fa, 0x42b2986c, 0xdbbbc9d6, 0xacbcf940, 0x32d86ce3,
        0x45df5c75, 0xdcd60dcf, 0xabd13d59, 0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423,
        0xcfba9599, 0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924, 0x2f6f7c87, 0x58684c11, 0xc1611dab,
        0xb6662d3d, 0x76dc4190, 0x01db7106, 0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
        0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d, 0x91646c97, 0xe6635c01, 0x6b6b51f4,
        0x1c6c6162, 0x856530d8, 0xf262004e, 0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950,
        0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65, 0x4db26158, 0x3ab551ce, 0xa3bc0074,
        0xd4bb30e2, 0x4adfa541, 0x3dd895d7, 0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
        0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa, 0xbe0b1010, 0xc90c2086, 0x5768b525,
        0x206f85b3, 0xb966d409, 0xce61e49f, 0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81,
        0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a, 0xead54739, 0x9dd277af, 0x04db2615,
        0x73dc1683, 0xe3630b12, 0x94643b84, 0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
        0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb, 0x196c3671, 0x6e6b06e7, 0xfed41b76,
        0x89d32be0, 0x10da7a5a, 0x67dd4acc, 0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5, 0xd6d6a3e8, 0xa1d1937e,
        0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b, 0xd80d2bda, 0xaf0a1b4c, 0x36034af6,
        0x41047a60, 0xdf60efc3, 0xa867df55, 0x316e8eef, 0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
        0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe, 0xb2bd0b28, 0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7,
        0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d, 0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f,
        0x72076785, 0x05005713, 0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38, 0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7,
        0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242, 0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
        0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69, 0x616bffd3, 0x166ccf45, 0xa00ae278,
        0xd70dd2ee, 0x4e048354, 0x3903b3c2, 0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc,
        0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9, 0xbdbdf21c, 0xcabac28a, 0x53b39330,
        0x24b4a3a6, 0xbad03605, 0xcdd70693, 0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
        0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d
      };

#define UPDC32(octet, crc) (crc_32_tab[((crc) ^ ((uint8_t)(octet))) & 0xff] ^ ((crc) >> 8))

uint32_t Utilities::computeCrc32(const uint8_t * const start, const uint8_t * const end)
{
    uint32_t oldcrc32;
    oldcrc32 = 0xFFFFFFFF;
    for (const auto * buf = start; buf != end; ++buf) {
        oldcrc32 = UPDC32(*buf, oldcrc32);
    }

    return ~oldcrc32;
}

bool Utilities::setPngResolutionDpi(std::vector<BYTE> & imageStream, const double resolution_dpi)
{
    static const auto chunkLenBytes = toBytes(9);
    auto resolutionBytes = toBytes(roundInteger(resolution_dpi * 1000.0 / 25.4));

    static const std::vector<BYTE> s_physStr { 'p', 'H', 'Y', 's' };

    auto pHYsChunk = chunkLenBytes;
    pHYsChunk.reserve(21); // 4 + 4 + 9 + 4

    pHYsChunk.insert(pHYsChunk.end(), s_physStr.begin(), s_physStr.end());
    pHYsChunk.insert(pHYsChunk.end(), resolutionBytes.begin(), resolutionBytes.end());
    pHYsChunk.insert(pHYsChunk.end(), resolutionBytes.begin(), resolutionBytes.end());
    pHYsChunk.push_back(1); // Unit is the meter

    const auto crcValue = computeCrc32(&pHYsChunk[4], &pHYsChunk[4] + 13);
    auto crcBytes = toBytes(static_cast<uint32_t>(crcValue));

    pHYsChunk.insert(pHYsChunk.end(), crcBytes.begin(), crcBytes.end());

    static const std::vector<BYTE> s_idat { 'I', 'D', 'A', 'T' };
    const auto itIdat = search(imageStream.begin(), imageStream.end(), s_idat.begin(), s_idat.end());
    if (itIdat == imageStream.end()) {
        throw std::runtime_error("IDAT section not found in PNG byte stream");
    }

    // Check if we already have a pHYs chunk in the stream (compare first 8 bytes)
    const auto itPhys = search(imageStream.begin(), imageStream.end(), pHYsChunk.begin(), pHYsChunk.begin() + 8);
    if (itPhys != imageStream.end()) {
        // We found it, replace existing content
        std::copy(pHYsChunk.begin(), pHYsChunk.end(), itPhys);
        return false;
    }
    assert(pHYsChunk.size() == 21);

    // Insert the chunk in the stream
    imageStream.insert(itIdat - 4, pHYsChunk.begin(), pHYsChunk.end());
    return true;
}

std::string Utilities::encodeImageAsPng(const cv::Mat & image, const bool encodeBase64, double resolution_dpi)
{
    std::vector<BYTE> pictureData;
    imencode(".png", image, pictureData);
    if (resolution_dpi > 0) {
        setPngResolutionDpi(pictureData, resolution_dpi);
    }
    if (encodeBase64) {
        return base64Encode(pictureData);
    }
    return std::string(pictureData.begin(), pictureData.end());
}

std::string Utilities::serializeJson(rapidjson::Document & d, const bool pretty)
{
    rapidjson::StringBuffer buffer;
    if (pretty) {
        rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
        d.Accept(writer);
    } else {
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        d.Accept(writer);
    }
    return std::string(buffer.GetString());
}

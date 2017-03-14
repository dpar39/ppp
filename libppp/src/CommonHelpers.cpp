#include "CommonHelpers.h"

#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <fstream>
#include <mutex>

#ifdef POCO_STATIC
#include <Poco/Path.h>
#include <Poco/File.h>
#else
#include <filesystem>
#endif



static uint8_t fromChar(char ch)
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

std::string base64Decode(const std::string &base64Str)
{
    uint8_t charBlock4[4], byteBlock3[3];
    std::string result;
    result.reserve(base64Str.size() * 3 / 4);

    auto i = 0;
    for (auto ch64 : base64Str)
    {
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
            result.append(reinterpret_cast<char *>(byteBlock3), 3);
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
        result.append(reinterpret_cast<char *>(byteBlock3), i - 1);
    }

    return result;
}

std::shared_ptr<cv::CascadeClassifier> CommonHelpers::loadClassifierFromBase64(const std::string &haarCascadeBase64Data)
{
    auto xmlHaarCascade = base64Decode(haarCascadeBase64Data);
    auto classifier = std::make_shared<cv::CascadeClassifier>();
    // Workaround until there I find a way to convert to the new 
    // format that is accepted as an in-memory FileNode
    // Implementation should look like this:

    //cv::FileStorage fs(xmlHaarCascade, cv::FileStorage::READ | cv::FileStorage::MEMORY);
    //auto fnode = fs.getFirstTopLevelNode();
    //classifier->read(fnode);
    //return classifier;
    static std::mutex g_mutex;
    std::lock_guard<std::mutex> lg(g_mutex);
    std::string tmpFile = "cascade.xml";
    {
        std::fstream oss(tmpFile, std::ios::out | std::ios::trunc);
        oss.write(xmlHaarCascade.c_str(), xmlHaarCascade.size());
    }
    classifier->load(tmpFile);
    return classifier;
}

uint32_t CommonHelpers::crc32(uint32_t crc, const uint8_t* begin, const uint8_t* end)
{
    /* Table of CRCs of all 8-bit messages. */
    static uint32_t s_crcTable[256];
    static std::once_flag s_crcComputeFlag;
    std::call_once(s_crcComputeFlag, []()
    {
        unsigned long c;
        int n, k;
        for (n = 0; n < 256; n++)
        {
            c = static_cast<unsigned long>(n);
            for (k = 0; k < 8; k++)
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
    while(it != end)
    {
        crc = s_crcTable[(crc ^ *it++) & 0xff] ^ (crc >> 8);
    }
    return crc;
}

#if 0
std::shared_ptr<cv::CascadeClassifier> CommonHelpers::loadClassifierFromFile(const std::string &haarCascadeDir, const std::string &haarCascadeFile)
{
    auto classifier = std::make_shared<cv::CascadeClassifier>();
#ifdef USE_POCO
    auto haarCascadeFilePathStr = Poco::Path(haarCascadeDir).append(haarCascadeFile).toString();
    if (!Poco::File(haarCascadeFilePathStr).exists())
#else
    using namespace std::tr2::sys;
    const path haarCascadeFilePath((path(haarCascadeDir) / path(haarCascadeFile)).string());
    auto haarCascadeFilePathStr = haarCascadeFilePath.string();
    if (!exists(haarCascadeFilePath))
#endif
    {
        throw std::runtime_error("Classifier file does not exist: " + haarCascadeFilePathStr);
    }
    if (!classifier->load(haarCascadeFilePathStr))
    {
        throw std::runtime_error("Unable to load classifier from file: " + haarCascadeFilePathStr);
    }
    return classifier;
}

cv::Rect CommonHelpers::detectObjectWithHaarCascade(const cv::Mat &image, cv::CascadeClassifier *cc, int dx /*= 0*/, int dy /*= 0*/)
{
    // Convert to gray scale if not done
    cv::Mat grayImage;
    if (image.channels() != 1)
    {
        cv::cvtColor(image, grayImage, CV_BGR2GRAY);
    }
    else
    {
        grayImage = image;
    }
    std::vector<cv::Rect> results;
    // Detect biggest object in the image
    cc->detectMultiScale(grayImage, results, 1.05, 3,
                         CV_HAAR_SCALE_IMAGE | CV_HAAR_FIND_BIGGEST_OBJECT);
    if (results.empty() || results.size() > 1)
        return cv::Rect();
    auto rect(results.front());
    rect.x += dx;
    rect.y += dy;
    return rect;
}
#endif


/* Update a running CRC with the bytes buf[0..len-1]--the CRC
should be initialized to all 1's, and the transmitted value
is the 1's complement of the final running CRC (see the
crc() routine below)). */
uint32_t updateCrc(uint32_t crc, unsigned char *buf, size_t len)
{
    /* Table of CRCs of all 8-bit messages. */
    static uint32_t s_crcTable[256];
    static std::once_flag s_crcComputeFlag;
    std::call_once(s_crcComputeFlag, []()
    {
        unsigned long c;
        int n, k;
        for (n = 0; n < 256; n++)
        {
            c = static_cast<unsigned long>(n);
            for (k = 0; k < 8; k++)
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

double CommonHelpers::toMM(double v, const std::string& units)
{
    if (units == "mm")
    {
        return v;
    }
    if (units == "inch")
    {
        return v*25.4;
    }    
    if (units == "cm")
    {
        return v*10.0;
    }
    throw std::runtime_error("Unknown input units when creating the photo standard definition");
}

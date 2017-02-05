#include "CommonHelpers.h"

#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#ifdef POCO_STATIC
#include <Poco/Path.h>
#include <Poco/File.h>
#else
#include <filesystem>
#endif

#include <common.h>

std::shared_ptr<cv::CascadeClassifier> CommonHelpers::loadClassifier(const std::string& haarCascadeDir, const std::string& haarCascadeFile)
{
    auto classifier = std::make_shared<cv::CascadeClassifier>();
#ifdef POCO_STATIC
    auto haarCascadeFilePathStr = resolvePath(Poco::Path(haarCascadeDir).append(haarCascadeFile).toString());
    if (!Poco::File(haarCascadeFilePathStr).exists())
#else
    using namespace std::tr2::sys;
    const path haarCascadeFilePath(resolvePath((path(haarCascadeDir) / path(haarCascadeFile)).string()));
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

cv::Rect CommonHelpers::detectObjectWithHaarCascade(const cv::Mat& image, cv::CascadeClassifier* cc, int dx /*= 0*/, int dy /*= 0*/)
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


/* Table of CRCs of all 8-bit messages. */
unsigned long crc_table[256];

/* Flag: has the table been computed? Initially false. */
int crc_table_computed = 0;

/* Make the table for a fast CRC. */
void make_crc_table(void)
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
        crc_table[n] = c;
    }
    crc_table_computed = 1;
}

/* Update a running CRC with the bytes buf[0..len-1]--the CRC
should be initialized to all 1's, and the transmitted value
is the 1's complement of the final running CRC (see the
crc() routine below)). */

unsigned long update_crc(unsigned long crc, unsigned char *buf, size_t len)
{
    auto c = crc;
    int n;

    if (!crc_table_computed)
    {
        make_crc_table();
    }
    for (n = 0; n < len; n++)
    {
        c = crc_table[(c ^ buf[n]) & 0xff] ^ (c >> 8);
    }
    return c;
}

/* Return the CRC of the bytes buf[0..len-1]. */
unsigned long crc(unsigned char *buf, int len)
{
    return update_crc(0xffffffffL, buf, len) ^ 0xffffffffL;
}
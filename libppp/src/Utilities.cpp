#include "Utilities.h"

#include <fstream>
#include <mutex>
#include <numeric>

#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/imgproc/imgproc.hpp>

namespace cv
{
    FWD_DECL(CascadeClassifier)
}


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

std::vector<BYTE> Utilities::base64Decode(const char *base64Str, size_t base64Len)
{
    BYTE charBlock4[4], byteBlock3[3];
    std::vector<BYTE> result;
    result.reserve(base64Len * 3 / 4);
    auto i = 0;
    for (size_t k = 0; k < base64Len; ++k)
    {
        auto ch64 = base64Str[k];
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



std::string Utilities::base64Encode(const std::vector<BYTE>& rawStr)
{
    static const std::string Base64CharSet =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";

    auto byteIter = rawStr.data();
    auto bufferSize = rawStr.size();
    std::string result;
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
                result += Base64CharSet[charArray4[i]];
            }
            i = 0;
        }
    }

    if (i > 0)
    {
        for (j = i; j < 3; j++)
        {
            charArray3[j] = '\0';
        }
        charArray4[0] = (charArray3[0] & 0xfc) >> 2;
        charArray4[1] = ((charArray3[0] & 0x03) << 4) + ((charArray3[1] & 0xf0) >> 4);
        charArray4[2] = ((charArray3[1] & 0x0f) << 2) + ((charArray3[2] & 0xc0) >> 6);
        charArray4[3] = charArray3[2] & 0x3f;
        for (j = 0; j < i + 1; j++)
        {
            result += Base64CharSet[charArray4[j]];
        }

        while (i++ < 3)
        {
            result += '=';
        }
    }
    return result;
}

cv::CascadeClassifierSPtr Utilities::loadClassifierFromBase64(const char *haarCascadeBase64Data)
{
    auto xmlHaarCascade = base64Decode(haarCascadeBase64Data, strlen(haarCascadeBase64Data));
    auto classifier = std::make_shared<cv::CascadeClassifier>();
    // Workaround until there I find a way to convert to the new
    // format that is accepted as an in-memory FileNode
    // Implementation should look like this:

    std::string s(xmlHaarCascade.begin(), xmlHaarCascade.end());
    try
    {
        cv::FileStorage fs(s, cv::FileStorage::READ | cv::FileStorage::MEMORY);
        classifier->read(fs.getFirstTopLevelNode());
        if (classifier->empty())
        {
            throw std::runtime_error("Failed to load classifier from configuration");
        }
        return classifier;

    }
    catch(std::exception &e)
    {
        throw e;
    }
    //static std::mutex g_mutex;
    //std::lock_guard<std::mutex> lg(g_mutex);
    //std::string tmpFile = "cascade.xml";
    //{
    //    std::fstream oss(tmpFile, std::ios::out | std::ios::trunc);
    //    oss.write(reinterpret_cast<char *>(xmlHaarCascade.data()), xmlHaarCascade.size());
    //}
    //classifier->load(tmpFile);
    //return classifier;
}

uint32_t Utilities::crc32(uint32_t crc, const uint8_t* begin, const uint8_t* end)
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
    while (it != end)
    {
        crc = s_crcTable[(crc ^ *it++) & 0xff] ^ (crc >> 8);
    }
    return crc;
}


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

double Utilities::toMM(double v, const std::string& units)
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

std::pair<cv::Point2d, cv::Point2d> Utilities::pointsAtDistanceNormalToCentreOf(cv::Point2d p1, cv::Point2d p2, double d)
{
    if (p1 == p2)
    {
        throw std::runtime_error("Input points cannot be equal");
    }

    auto p0 = (p1 + p2) / 2.0;
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
        auto dx = d / sqrt(1 + m * m);
        if (m < 0) dx = -dx;
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
    auto ratio = dist / cv::norm(p1 - p0);
    return p0 + (p1 - p0) * ratio;
}

std::vector<cv::Point2d> Utilities::contourLineIntersection(const std::vector<cv::Point> contour, cv::Point2d pline1, cv::Point2d pline2)
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

int Utilities::kittlerOptimumThreshold(std::vector<double> P, float mu)
{
    int t, i, first = 0, last = 0, opt_threshold = 0, set = 0;
    double q1[255], q2[255], mu1[255], mu2[255], var1[255], var2[255], H[255];
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
    var1[first] = (first - mu1[first]) * (first - mu1[first]) * P[first] / q1[first];
    var2[first] = 0;
    for (i = first; i < last; i++)
    {
        var2[first] += (i - mu2[first]) * (i - mu2[first]) * P[i] / q2[first];
    }

    /* RUN THROUGH THE THRESHOLDS FOR OPTIMA */
    for (t = first + 1; t < last; t++)
    {
        q1[t] = q1[t - 1] + P[t];
        q2[t] = 1 - q1[t];
        mu1[t] = (q1[t - 1] * mu1[t - 1] + static_cast<double>(t) * P[t]) / q1[t];
        mu2[t] = (mu - q1[t] * mu1[t]) / q2[t];

        /* ENERGY FUNCTION */
        var1[t] = (q1[t - 1] * (var1[t - 1] + (mu1[t - 1] - mu1[t]) * (mu1[t - 1] - mu1[t])) + P[t] * (t - mu1[t]) * (t - mu1[t])) / q1[t];
        var2[t] = (q2[t - 1] * (var2[t - 1] + (mu2[t - 1] - mu2[t]) * (mu2[t - 1] - mu2[t])) - P[t] * (t - mu2[t]) * (t - mu2[t])) / q2[t];

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

cv::Mat Utilities::selfCoefficientImage(const cv::Mat& inputImg, int kernelSize)
{
    if ( kernelSize < 3 || kernelSize % 2 == 0)
    {
        throw std::logic_error("Kernel size should be a positive odd number greater or equal to 3");
    }

    auto halfsize = static_cast<int>(ceil(kernelSize / 2));

    auto sigma = kernelSize / 5.0;

    std::vector<double> kernel;

    for (auto i = 0; i < kernelSize; i++)
    {
        for (auto j = 0; j < kernelSize; j++)
        {
            auto x_value = -halfsize + i;
            auto y_value = -halfsize + j;
            kernel.push_back(exp(-(static_cast<double>(x_value * x_value + y_value * y_value) / sigma)));
        }
    }

    auto outputImg = cv::Mat(inputImg.size(), inputImg.type(), cv::Scalar::all(0));

    if (inputImg.channels() != 1)
    {
        throw std::runtime_error("Image should be grayscale");
    }

    auto height = inputImg.size().height;
    auto width = inputImg.size().width;

    auto windowSizeSqr = kernelSize*kernelSize;

    std::vector<double> windowPixelData(windowSizeSqr);
    std::vector<double> wKernel(windowSizeSqr);

    for (auto r = 0; r < height; ++r)
    {
        for (auto c = 0; c < width; ++c)
        {
            auto regionMean = 0.0;
            auto idx = 0;

            // FIRST LOOP: compute region_mean
            for (auto y = -halfsize; y <= halfsize; y++)
            {
                auto row = r - y;
                if (row < 0)
                {
                    row = abs(row) - 1;
                }
                if (row >= height)
                {
                    row = (2 * height) - row - 1;
                }
                for (auto x = -halfsize; x <= halfsize; x++)
                {
                    auto col = c - x;
                    if (col < 0)
                    {
                        col = abs(col) - 1;
                    }
                    if (col >= width)
                    {
                        col = (2 * width) - col - 1;
                    }
                    auto v = inputImg.at<uchar>(row, col);
                    windowPixelData[idx++] = v;
                    regionMean += v;
                }
            }

            regionMean /= windowSizeSqr;

            // SECOND LOOP: count number of pixels bigger/smaller than the mean
            auto overMeanCnt = std::count_if(windowPixelData.begin(), windowPixelData.end(), [regionMean](double v) {return v > regionMean; });
            auto underMeanCnt = windowSizeSqr - overMeanCnt;
            auto above = overMeanCnt > underMeanCnt;

            // THIRD LOOP : update filter weights
            auto weightedKernelSum = 0.0;
            std::transform( windowPixelData.begin(), windowPixelData.end(), kernel.begin(), wKernel.begin(), [regionMean, above, &weightedKernelSum](double v, double kernelValue)
            {
                if ((v > regionMean && above == false) || (v < regionMean && above == true))
                {
                    return 0.0;
                }
                else
                {
                    weightedKernelSum += kernelValue;
                    return kernelValue;
                }
            });
            // normalize s.t. sum(filter coeff) = 1
            std::transform(wKernel.begin(), wKernel.end(), wKernel.begin(), [weightedKernelSum](double v) {return v / weightedKernelSum; });

            // FOURTH LOOP : convolve
            auto pixelValue = std::inner_product(windowPixelData.begin(), windowPixelData.end(), wKernel.begin(), 0.0);

            outputImg.at<uchar>(r, c) = pixelValue;
        }
    }

    return outputImg;
}

cv::Point Utilities::convert(const dlib::point& pt)
{
    return cv::Point2d(pt.x(), pt.y());
}

cv::Rect2d Utilities::convert(const dlib::rectangle& r)
{
    return cv::Rect2d(r.left(), r.top(), r.width(), r.height());
}

dlib::rectangle Utilities::convert(const cv::Rect2d& r)
{
    return dlib::rectangle(r.x, r.y, r.x + r.width, r.y + r.height);
}

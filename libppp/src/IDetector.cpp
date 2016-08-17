#include "IDetector.h"

#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#ifdef POCO_SATIC
#include <Poco/Path.h>
#include <Poco/File.h>
#else
#include <filesystem>
#endif

#include <common.h>

std::shared_ptr<cv::CascadeClassifier> IDetector::loadClassifier(const std::string& haarCascadeDir, const std::string& haarCascadeFile)
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

cv::Rect IDetector::detectObjectWithHaarCascade(const cv::Mat& image, cv::CascadeClassifier* cc, int dx /*= 0*/, int dy /*= 0*/)
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

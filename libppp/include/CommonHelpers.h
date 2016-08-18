#pragma once
#include <memory>

#include <opencv2/core/core.hpp>

namespace cv
{
    class CascadeClassifier;
}

class CommonHelpers
{
public:
    /*!
    Loads a classifier from file
    !*/
    static std::shared_ptr<cv::CascadeClassifier> loadClassifier(const std::string &haarCascadeDir, const std::string &haarCascadeFile);

    
    static cv::Rect detectObjectWithHaarCascade(const cv::Mat& image, cv::CascadeClassifier *cc, int dx = 0, int dy = 0);
};


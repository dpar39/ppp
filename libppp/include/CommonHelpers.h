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
    /*!@brief Loads a cascade classifier from file
    *  @param[in] Directory containing the classifier file to load
    *  @param[in] File name of the classifier file to load
    *  @returns The classifier loaded into memory
    !*/
    static std::shared_ptr<cv::CascadeClassifier> loadClassifier(const std::string &haarCascadeDir, const std::string &haarCascadeFile);

    /*!@brief Detects an object in an image using the classifier passed as parameter
    *  The rectangle returned is shifted by the coordinates dx and dy
    !*/
    static cv::Rect detectObjectWithHaarCascade(const cv::Mat& image, cv::CascadeClassifier *cc, int dx = 0, int dy = 0);
};

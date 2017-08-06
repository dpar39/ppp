#include "CommonHelpers.h"
#include "FaceDetector.h"
#include "LandMarks.h"
#include "Utilities.h"

#include <vector>

#include <opencv2/core/mat.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/objdetect/objdetect.hpp>

using namespace std;
using namespace cv;

bool FaceDetector::detectLandMarks(const cv::Mat& inputPicture, LandMarks& landmarks)
{
    // Configuration
    const auto minFaceRatio = 0.15;
    const auto maxFaceRatio = 0.85;

    // Calculate search domain on the image
    Size minFaceSize, maxFaceSize, imgSize = inputPicture.size();
    calculateScaleSearch(imgSize, minFaceRatio, maxFaceRatio, minFaceSize, maxFaceSize);

    auto grayImage = inputPicture;

    if (inputPicture.channels() != 1)
    {
        cv::cvtColor(inputPicture, grayImage, CV_BGR2GRAY);
    }

    vector<Rect> facesRects;
    vector<int> rejectLevels;
    vector<double> levelWeights;
    m_pFaceCascadeClassifier->detectMultiScale(grayImage, facesRects, 1.05, 3,
        CV_HAAR_SCALE_IMAGE | CV_HAAR_FIND_BIGGEST_OBJECT,
        minFaceSize, maxFaceSize);

    if (facesRects.size() == 0)
    {
        return false;
    }
    landmarks.vjFaceRect = facesRects.front();
    return true;
}


void FaceDetector::calculateScaleSearch(const Size& inputImageSize, double minFaceRatio, double maxFaceRatio, Size& minFaceSize, Size& maxFaceSize) const
{
    auto dim = std::minmax(inputImageSize.height, inputImageSize.width);
    auto minFaceSizePix = static_cast<int>(dim.first * minFaceRatio);
    auto maxFaceSizePix = static_cast<int>(dim.second * maxFaceRatio);
    minFaceSize = Size(minFaceSizePix, minFaceSizePix);
    maxFaceSize = Size(maxFaceSizePix, maxFaceSizePix);
}

void FaceDetector::configure(rapidjson::Value& cfg)
{
    auto xmlBase64Data(cfg["faceDetector"]["haarCascade"]["data"].GetString());
    m_pFaceCascadeClassifier = Utilities::loadClassifierFromBase64(xmlBase64Data);
}

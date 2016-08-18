#include "FaceDetector.h"
#include "LandMarks.h"

#include <vector>

#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/core/mat.hpp>

#include <opencv2/imgproc/imgproc.hpp>
#include <EyeDetector.h>
#include <CommonHelpers.h>

using namespace std;
using namespace cv;

bool FaceDetector::detectLandMarks(const cv::Mat& inputPicture, LandMarks& landmarks)
{
    // Configuration
    double minFaceRatio = 0.15;
    double maxFaceRatio = 0.85;

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
                                               CV_HAAR_SCALE_IMAGE | CV_HAAR_FIND_BIGGEST_OBJECT, minFaceSize, maxFaceSize);

    //m_pCascadeClassifier->detectMultiScale(grayImage, facesRects,
    //rejectLevels, levelWeights, 1.05, 3, CV_HAAR_SCALE_IMAGE , minFaceSize , maxFaceSize, true);

    if (facesRects.size() == 0)
    {
        return false;
    }
    landmarks.vjFaceRect = facesRects.front();
    return true;
}


void FaceDetector::calculateScaleSearch(const Size& inputImageSize, double minFaceRatio, double maxFaceRatio, Size& minFaceSize, Size& maxFaceSize)
{
    auto dim = std::minmax(inputImageSize.height, inputImageSize.width);
    int minFaceSizePix = static_cast<int>(dim.first * minFaceRatio);
    int maxFaceSizePix = static_cast<int>(dim.second * maxFaceRatio);
    minFaceSize = Size(minFaceSizePix, minFaceSizePix);
    maxFaceSize = Size(maxFaceSizePix, maxFaceSizePix);
}

void FaceDetector::configure(rapidjson::Value& cfg)
{
    const string haarCascadeDir(cfg["haarCascadeDir"].GetString());
    const string haarCascadeFile(cfg["faceDetector"]["haarCascade"].GetString());
    m_pFaceCascadeClassifier = CommonHelpers::loadClassifier(haarCascadeDir, haarCascadeFile);
}

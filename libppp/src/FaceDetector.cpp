#include "FaceDetector.h"
#include "LandMarks.h"
#include "Utilities.h"

#include <vector>

#include <opencv2/core/mat.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/objdetect/objdetect.hpp>
#include <dlib/opencv/cv_image.h>

using namespace std;
using namespace cv;

bool FaceDetector::detectLandMarks(const cv::Mat& inputPicture, LandMarks& landmarks)
{
    if (m_useDlibFaceDetection)
    {
        using namespace dlib;
        array2d<uint8_t> dlibImage;
        assign_image(dlibImage, cv_image<uint8_t>(inputPicture));

        auto dets = (*m_frontalFaceDetector)(dlibImage);

        if (dets.empty())
        {
            return false; // No face was found
        }

        auto &faceRect = dets.front();
        if (dets.size() > 1)
        {
            auto biggestFacePtr = std::max_element(dets.begin(), dets.end(), [](const dlib::rectangle &r1, const dlib::rectangle &r2)
            {
                return r1.area() < r2.area();
            });
            faceRect = *biggestFacePtr;
        }
        landmarks.vjFaceRect = Utilities::convert(faceRect);
    }
    else
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
            cv::cvtColor(inputPicture, grayImage, COLOR_BGR2GRAY);
        }

        vector<Rect> facesRects;
        vector<int> rejectLevels;
        vector<double> levelWeights;
        m_pFaceCascadeClassifier->detectMultiScale(grayImage, facesRects, 1.05, 3,
            CASCADE_SCALE_IMAGE | CASCADE_FIND_BIGGEST_OBJECT,
            minFaceSize, maxFaceSize);

        if (facesRects.size() == 0)
        {
            return false;
        }
        landmarks.vjFaceRect = facesRects.front();
    }
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

void FaceDetector::configure(rapidjson::Value& config)
{
    auto xmlBase64Data(config["faceDetector"]["haarCascade"]["data"].GetString());
    m_pFaceCascadeClassifier = Utilities::loadClassifierFromBase64(xmlBase64Data);

    m_useDlibFaceDetection = config["useDlibFaceDetection"].GetBool();

    if (m_useDlibFaceDetection)
    {
        m_frontalFaceDetector = std::make_shared<dlib::frontal_face_detector>(dlib::get_frontal_face_detector());
    }
}

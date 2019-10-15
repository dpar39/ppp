#include "FaceDetector.h"
#include "LandMarks.h"
#include "Utilities.h"

#include <vector>

#include <dlib/opencv/cv_image.h>
#include <opencv2/core/mat.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/objdetect/objdetect.hpp>

using namespace std;
using namespace cv;

bool FaceDetector::detectLandMarks(const Mat & inputImage, LandMarks & landmarks)
{
    if (m_useDlibFaceDetection)
    {
        using namespace dlib;
        array2d<uint8_t> dlibImage;
        assign_image(dlibImage, cv_image<uint8_t>(inputImage));

        auto dets = (*m_frontalFaceDetector)(dlibImage);

        if (dets.empty())
        {
            return false; // No face was found
        }

        auto & faceRect = dets.front();
        if (dets.size() > 1)
        {
            const auto biggestFacePtr = std::max_element(dets.begin(),
                                                         dets.end(),
                                                         [](const dlib::rectangle & r1, const dlib::rectangle & r2) {
                                                             return r1.area() < r2.area();
                                                         });
            faceRect = *biggestFacePtr;
        }

        landmarks.vjFaceRect = Rect2d(faceRect.left(), faceRect.top(), faceRect.width(), faceRect.height());
        landmarks.imageRotation = 0; // TODO: Add rotation search to this one as well
        return true;
    }

    // Configuration
    const auto minFaceRatio = 0.15;
    const auto maxFaceRatio = 0.85;

    // Calculate search domain on the image
    const auto imgSize = inputImage.size();
    const auto h = imgSize.height;
    const auto w = imgSize.width;

    auto grayImage = inputImage;
    if (inputImage.channels() != 1)
    {
        cvtColor(inputImage, grayImage, COLOR_BGR2GRAY);
    }

    for (const auto angle : { 0, 90, -90, 180 })
    {
        // Let's rotate the image to see if we can find a face in it
        auto rotatedImage = Utilities::rotateImage(grayImage, angle);
        Size minFaceSize, maxFaceSize;
        calculateScaleSearch(imgSize, minFaceRatio, maxFaceRatio, minFaceSize, maxFaceSize);

        vector<Rect> facesRects;
        vector<int> rejectLevels;
        vector<double> levelWeights;
        m_pFaceCascadeClassifier->detectMultiScale(rotatedImage,
                                                   facesRects,
                                                   1.05,
                                                   4,
                                                   CASCADE_SCALE_IMAGE | CASCADE_FIND_BIGGEST_OBJECT,
                                                   minFaceSize,
                                                   maxFaceSize);

        if (!facesRects.empty())
        {
            landmarks.vjFaceRect = facesRects.front();
            landmarks.imageRotation = angle;
            return true;
        }
    }
    return false;
}

void FaceDetector::calculateScaleSearch(const Size & inputImageSize,
                                        const double minFaceRatio,
                                        const double maxFaceRatio,
                                        Size & minFaceSize,
                                        Size & maxFaceSize) const
{
    const auto dim = std::minmax(inputImageSize.height, inputImageSize.width);
    const auto minFaceSizePix = static_cast<int>(dim.first * minFaceRatio);
    const auto maxFaceSizePix = static_cast<int>(dim.second * maxFaceRatio);
    minFaceSize = Size(minFaceSizePix, minFaceSizePix);
    maxFaceSize = Size(maxFaceSizePix, maxFaceSizePix);
}

void FaceDetector::configure(rapidjson::Value & config)
{
    const auto xmlBase64Data(config["faceDetector"]["haarCascade"]["data"].GetString());
    m_pFaceCascadeClassifier = Utilities::loadClassifierFromBase64(xmlBase64Data);

    m_useDlibFaceDetection = config["useDlibFaceDetection"].GetBool();

    if (m_useDlibFaceDetection)
    {
        m_frontalFaceDetector = std::make_shared<dlib::frontal_face_detector>(dlib::get_frontal_face_detector());
    }
}

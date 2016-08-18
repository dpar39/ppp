#include "PppEngine.h"
#include "LandMarks.h"

#include "IDetector.h"
#include "IPhotoPrintMaker.h"

// Implementations injected by default
#include "EyeDetector.h"
#include "FaceDetector.h"
#include "LipsDetector.h"
#include "PhotoPrintMaker.h"


#include "PhotoStandard.h"
#include "CanvasDefinition.h"
#include <Geometry.h>

#include <opencv2/imgproc/imgproc.hpp>

using namespace std;

PppEngine::PppEngine(std::shared_ptr<IDetector> pFaceDetector /*= nullptr*/
                     , std::shared_ptr<IDetector> pEyesDetector /*= nullptr*/
                     , std::shared_ptr<IDetector> pLipsDetector /*= nullptr*/
                     , std::shared_ptr<IPhotoPrintMaker> pPhotoPrintMaker /*= nullptr*/)
    : m_pFaceDetector(pFaceDetector ? pFaceDetector : make_shared<FaceDetector>())
      , m_pEyesDetector(pEyesDetector ? pEyesDetector : make_shared<EyeDetector>())
      , m_pLipsDetector(pLipsDetector ? pLipsDetector : make_shared<LipsDetector>())
      , m_pPhotoPrintMaker(pPhotoPrintMaker ? pPhotoPrintMaker : make_shared<PhotoPrintMaker>())
{
    // Nothing to do here, just initialise stuffs
}


void PppEngine::configure(rapidjson::Value& config)
{
    m_pFaceDetector->configure(config);
    m_pEyesDetector->configure(config);
    m_pLipsDetector->configure(config);
}

bool PppEngine::detectLandMarks(LandMarks& landMarks)
{
    // Convert the image to gray scale as needed by some algorithms
    cv::Mat grayImage;
    cv::cvtColor(m_inputImage, grayImage, CV_BGR2GRAY);

    // Detect the face
    if (!m_pFaceDetector->detectLandMarks(grayImage, landMarks))
    {
        return false;
    }

    // Detect the eye pupils
    if (!m_pEyesDetector->detectLandMarks(grayImage, landMarks))
    {
        return false;
    }

    // Detect mouth landmarks
    if (!m_pLipsDetector->detectLandMarks(m_inputImage, landMarks))
    {
        return false;
    }

    // Estimate chin and crown point (maths from existing landmarks)
    estimateHeadTopAndChinCorner(landMarks);

    return true;
}


cv::Mat PppEngine::createTiledPrint(PhotoStandard& ps, CanvasDefinition& canvas, cv::Point& crownMark, cv::Point& chinMark)
{
    auto croppedImage = m_pPhotoPrintMaker->cropPicture(m_inputImage, crownMark, chinMark, ps);
    auto tiledPrintPhoto = canvas.tileCroppedPhoto(ps, croppedImage);
    return tiledPrintPhoto;
}

void PppEngine::estimateHeadTopAndChinCorner(LandMarks& landMarks)
{
    // Using normalised distance to be the sum of the distance between eye pupils and the distance mouth to frown
    // Distance chin to crown is estimated as 1.7699 of that value with correlation 0.7954
    // Distance chin to frown is estimated as 0.8945 of that value with correlation 0.8426
    const auto chinCrownCoeff = 1.7699;
    const auto chinFrownCoeff = 0.8945;

    auto frownPointPix = (landMarks.eyeLeftPupil + landMarks.eyeRightPupil) / 2;
    auto mouthPointPix = (landMarks.lipLeftCorner + landMarks.lipRightCorner) / 2;
    auto normalisedDistancePixels = cv::norm(landMarks.eyeLeftPupil - landMarks.eyeRightPupil)
        + cv::norm(frownPointPix - mouthPointPix);

    auto chinFrownDistancePix = chinFrownCoeff * normalisedDistancePixels;
    auto chinCrownDistancePix = chinCrownCoeff * normalisedDistancePixels;

    landMarks.chinPoint = pointInLineAtDistance(frownPointPix, mouthPointPix, chinFrownDistancePix);
    landMarks.crownPoint = pointInLineAtDistance(landMarks.chinPoint, frownPointPix, chinCrownDistancePix);
}

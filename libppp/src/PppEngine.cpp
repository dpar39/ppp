#include "PppEngine.h"
#include "LandMarks.h"

// Implementations injected by default
#include "EyeDetector.h"
#include "FaceDetector.h"
#include "LipsDetector.h"
#include "ImageStore.h"
#include "PhotoPrintMaker.h"

#include "PhotoStandard.h"
#include "CanvasDefinition.h"
#include "Geometry.h"

#include <opencv2/imgproc/imgproc.hpp>
#include <iomanip>


using namespace std;
uint32_t  crc32(uint32_t crc, const uchar *buf, const uchar *bufferEnd);

PppEngine::PppEngine(std::shared_ptr<IDetector> pFaceDetector /*= nullptr*/
                     , std::shared_ptr<IDetector> pEyesDetector /*= nullptr*/
                     , std::shared_ptr<IDetector> pLipsDetector /*= nullptr*/
                     , std::shared_ptr<IPhotoPrintMaker> pPhotoPrintMaker /*= nullptr*/
                     , std::shared_ptr<IImageStore> pImageStore /*= nullptr*/)
    : m_pFaceDetector(pFaceDetector ? pFaceDetector : make_shared<FaceDetector>())
      , m_pEyesDetector(pEyesDetector ? pEyesDetector : make_shared<EyeDetector>())
      , m_pLipsDetector(pLipsDetector ? pLipsDetector : make_shared<LipsDetector>())
      , m_pPhotoPrintMaker(pPhotoPrintMaker ? pPhotoPrintMaker : make_shared<PhotoPrintMaker>())
      , m_pImageStore(pImageStore ? pImageStore : make_shared<ImageStore>())
{
}


void PppEngine::configure(rapidjson::Value& config)
{
    m_pFaceDetector->configure(config);
    m_pEyesDetector->configure(config);
    m_pLipsDetector->configure(config);

    size_t imageStoreSize = config["imageStoreSize"].GetInt();
    m_pImageStore->setStoreSize(imageStoreSize);

    m_pPhotoPrintMaker->configure(config);
}

void PppEngine::verifyImageExists(const std::string& imageKey) const
{
    if (!m_pImageStore->containsImage(imageKey))
    {
        throw std::runtime_error("Image with key='" + imageKey + "' not found!");
    }
}

std::string PppEngine::setInputImage(cv::Mat& inputImage)
{
    return m_pImageStore->setImage(inputImage);
}


bool PppEngine::detectLandMarks(const std::string& imageKey, LandMarks& landMarks)
{
    verifyImageExists(imageKey);
    // Convert the image to gray scale as needed by some algorithms

    const auto &inputImage = m_pImageStore->getImage(imageKey);
    cv::Mat grayImage;
    cv::cvtColor(inputImage, grayImage, CV_BGR2GRAY);

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
    if (!m_pLipsDetector->detectLandMarks(inputImage, landMarks))
    {
        return false;
    }

    // Estimate chin and crown point (maths from existing landmarks)
    estimateHeadTopAndChinCorner(landMarks);

    return true;
}


cv::Mat PppEngine::createTiledPrint(const std::string& imageKey, PhotoStandard& ps, CanvasDefinition& canvas,
                                    cv::Point& crownMark, cv::Point& chinMark)
{
    verifyImageExists(imageKey);

    const auto &inputImage = m_pImageStore->getImage(imageKey);

    auto croppedImage = m_pPhotoPrintMaker->cropPicture(inputImage, crownMark, chinMark, ps);

    auto tiledPrintPhoto = m_pPhotoPrintMaker->tileCroppedPhoto(canvas, ps, croppedImage);

    return tiledPrintPhoto;
}

void PppEngine::estimateHeadTopAndChinCorner(LandMarks& landMarks) const
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

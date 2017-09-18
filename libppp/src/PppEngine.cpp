#include "PppEngine.h"
#include "LandMarks.h"

// Implementations injected by default
#include "EyeDetector.h"
#include "FaceDetector.h"
#include "LipsDetector.h"
#include "CrownChinEstimator.h"

#include "ImageStore.h"
#include "PhotoPrintMaker.h"

#include "PhotoStandard.h"
#include "CanvasDefinition.h"

#include <opencv2/imgproc/imgproc.hpp>
#include <dlib/image_processing/shape_predictor.h>
#include <dlib/opencv/cv_image.h>

#include "Utilities.h"

using namespace std;

PppEngine::PppEngine(IDetectorSPtr pFaceDetector /*= nullptr*/
                     , IDetectorSPtr pEyesDetector /*= nullptr*/
                     , IDetectorSPtr pLipsDetector /*= nullptr*/
                     , ICrownChinEstimatorSPtr pCrownChinEstimator /* = nullptr*/
                     , IPhotoPrintMakerSPtr pPhotoPrintMaker /*= nullptr*/
                     , IImageStoreSPtr pImageStore /*= nullptr*/)
    : m_pFaceDetector(pFaceDetector ? pFaceDetector : make_shared<FaceDetector>())
      , m_pEyesDetector(pEyesDetector ? pEyesDetector : make_shared<EyeDetector>())
      , m_pLipsDetector(pLipsDetector ? pLipsDetector : make_shared<LipsDetector>())
      , m_pCrownChinEstimator(pCrownChinEstimator ? pCrownChinEstimator : make_shared<CrownChinEstimator>())
      , m_pPhotoPrintMaker(pPhotoPrintMaker ? pPhotoPrintMaker : make_shared<PhotoPrintMaker>())
      , m_pImageStore(pImageStore ? pImageStore : make_shared<ImageStore>())
      , m_useDlibLandmarkDetection(false)
{
}

bool PppEngine::configure(rapidjson::Value& config)
{
    m_pFaceDetector->configure(config);
    m_pEyesDetector->configure(config);
    m_pLipsDetector->configure(config);
    m_pCrownChinEstimator->configure(config);

    size_t imageStoreSize = config["imageStoreSize"].GetInt();
    m_pImageStore->setStoreSize(imageStoreSize);

    auto shapePredictorFile = config["shapePredictorFile"].GetString();

    m_useDlibLandmarkDetection = config["useDlibLandmarkDetection"].GetBool();

    m_pPhotoPrintMaker->configure(config);

    if (ifstream(shapePredictorFile).good())
    {
        m_shapePredictor = std::make_shared<dlib::shape_predictor>();
        dlib::deserialize(shapePredictorFile) >> *m_shapePredictor;
    }
    return true;
}

void PppEngine::verifyImageExists(const string& imageKey) const
{
    if (!m_pImageStore->containsImage(imageKey))
    {
        throw runtime_error("Image with key='" + imageKey + "' not found!");
    }
}

string PppEngine::setInputImage(const cv::Mat& inputImage) const
{
    return m_pImageStore->setImage(inputImage);
}


bool PppEngine::detectLandMarks(const string& imageKey, LandMarks& landMarks) const
{
    verifyImageExists(imageKey);
    // Convert the image to gray scale as needed by some algorithms

    const auto& inputImage = m_pImageStore->getImage(imageKey);
    cv::Mat grayImage;
    cvtColor(inputImage, grayImage, CV_BGR2GRAY);

    // Detect the face
    if (!m_pFaceDetector->detectLandMarks(grayImage, landMarks))
    {
        return false;
    }

    if (!m_useDlibLandmarkDetection)
    {
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
    }
    else
    {
        using namespace dlib;
        // Detect the face
        if (!m_pFaceDetector->detectLandMarks(grayImage, landMarks))
        {
            return false;
        }
        array2d<bgr_pixel> dlibImage;
        assign_image(dlibImage, cv_image<bgr_pixel>(inputImage));
        auto faceRect = Utilities::convert(landMarks.vjFaceRect);
        auto shape = (*m_shapePredictor)(dlibImage, faceRect);

        landMarks.lipLeftCorner = Utilities::convert(shape.part(49 - 1));
        landMarks.lipRightCorner = Utilities::convert(shape.part(55 - 1));
        landMarks.eyeLeftPupil = (Utilities::convert(shape.part(38 - 1)) + Utilities::convert(shape.part(39 - 1)) + Utilities::convert(shape.part(41 - 1)) + Utilities::convert(shape.part(42 - 1))) / 4;
        landMarks.eyeRightPupil = (Utilities::convert(shape.part(44 - 1)) + Utilities::convert(shape.part(45 - 1)) + Utilities::convert(shape.part(48 - 1)) + Utilities::convert(shape.part(47 - 1))) / 4;
        landMarks.vjFaceRect = Utilities::convert(faceRect);
        landMarks.chinPoint = Utilities::convert(shape.part(9 - 1));

        for (size_t i = 0; i < shape.num_parts(); ++i)
        {
            landMarks.allLandmarks.push_back(Utilities::convert(shape.part(i)));
        }
    }

    // Estimate chin and crown point (maths from existing landmarks)
    return m_pCrownChinEstimator->estimateCrownChin(landMarks);
}

cv::Mat PppEngine::createTiledPrint(const string& imageKey, PhotoStandard& ps, CanvasDefinition& canvas,
                                    cv::Point& crownMark, cv::Point& chinMark) const
{
    verifyImageExists(imageKey);

    const auto& inputImage = m_pImageStore->getImage(imageKey);

    auto croppedImage = m_pPhotoPrintMaker->cropPicture(inputImage, crownMark, chinMark, ps);

    auto tiledPrintPhoto = m_pPhotoPrintMaker->tileCroppedPhoto(canvas, ps, croppedImage);

    return tiledPrintPhoto;
}

#include "PppEngine.h"
#include "LandMarks.h"

// Implementations injected by default
#include "CrownChinEstimator.h"
#include "EyeDetector.h"
#include "FaceDetector.h"
#include "LipsDetector.h"

#include "ImageStore.h"
#include "PhotoPrintMaker.h"

#include "CanvasDefinition.h"
#include "PhotoStandard.h"

#include <dlib/image_processing/shape_predictor.h>
#include <dlib/opencv/cv_image.h>
#include <opencv2/imgproc/imgproc.hpp>

#include "Utilities.h"

using namespace std;

PppEngine::PppEngine(IDetectorSPtr pFaceDetector,
                     IDetectorSPtr pEyesDetector,
                     IDetectorSPtr pLipsDetector,
                     ICrownChinEstimatorSPtr pCrownChinEstimator,
                     IPhotoPrintMakerSPtr pPhotoPrintMaker,
                     IImageStoreSPtr pImageStore)
: m_pFaceDetector(pFaceDetector ? pFaceDetector : make_shared<FaceDetector>())
, m_pEyesDetector(pEyesDetector ? pEyesDetector : make_shared<EyeDetector>())
, m_pLipsDetector(pLipsDetector ? pLipsDetector : make_shared<LipsDetector>())
, m_pCrownChinEstimator(pCrownChinEstimator ? pCrownChinEstimator : make_shared<CrownChinEstimator>())
, m_pPhotoPrintMaker(pPhotoPrintMaker ? pPhotoPrintMaker : make_shared<PhotoPrintMaker>())
, m_pImageStore(pImageStore ? pImageStore : make_shared<ImageStore>())
, m_useDlibLandmarkDetection(false)
{
}

struct membuf : std::streambuf
{
    membuf(char const * base, const size_t size)
    {
        char * p(const_cast<char *>(base));
        this->setg(p, p, p + size);
    }
};
struct imemstream : virtual membuf, std::istream
{
    imemstream(char const * base, const size_t size)
    : membuf(base, size)
    , std::istream(static_cast<std::streambuf *>(this))
    {
    }
};

bool PppEngine::configure(const std::string & configString)
{

    rapidjson::Document config;
    config.Parse(configString.c_str());

    m_pFaceDetector->configure(config);
    m_pEyesDetector->configure(config);
    m_pLipsDetector->configure(config);
    m_pCrownChinEstimator->configure(config);

    const size_t imageStoreSize = config["imageStoreSize"].GetInt();
    m_pImageStore->setStoreSize(imageStoreSize);

    m_pPhotoPrintMaker->configure(config);

    m_useDlibLandmarkDetection = config["useDlibLandmarkDetection"].GetBool();

    if (m_useDlibLandmarkDetection)
    {
        auto & shapePredictor = config["shapePredictor"];
        m_shapePredictor = std::make_shared<dlib::shape_predictor>();
        const auto shapePredictorFile = shapePredictor["file"].GetString();
        if (ifstream(shapePredictorFile).good())
        {
            dlib::deserialize(shapePredictorFile) >> *m_shapePredictor;
        }
        else
        {
            const auto shapePredictorFileContent = shapePredictor["data"].GetString();
            auto spData = Utilities::base64Decode(shapePredictorFileContent, strlen(shapePredictorFileContent));
            imemstream stream(reinterpret_cast<char *>(&spData[0]), spData.size());
            deserialize(*m_shapePredictor, stream);
        }

        // Prepare landmark mapping
        set<int> missingLandMarks;
        auto array = shapePredictor["missingPoints"].GetArray();
        for (rapidjson::SizeType i = 0; i < array.Size(); i++)
        {
            missingLandMarks.insert(array[i].GetInt());
        }

        m_landmarkIndexMapping = {
            { LandMarkType::EYE_LEFT_PUPIL, std::vector<int> { 38, 39, 41, 42 } },
            { LandMarkType::EYE_RIGHT_PUPIL, std::vector<int> { 44, 45, 47, 48 } },
            { LandMarkType::LIPS_LEFT_CORNER, std::vector<int> { 49 } },
            { LandMarkType::LIPS_RIGHT_CORNER, std::vector<int> { 55 } },
            { LandMarkType::CHIN_LOWEST_POINT, std::vector<int> { 9 } },
        };

        for (auto & kv : m_landmarkIndexMapping)
        {
            for (auto & idx : kv.second)
            {
                const auto offset = std::distance(missingLandMarks.begin(), missingLandMarks.upper_bound(idx));
                idx -= offset + 1;
            }
        }
    }
    return true;
}

void PppEngine::verifyImageExists(const string & imageKey) const
{
    if (!m_pImageStore->containsImage(imageKey))
    {
        throw runtime_error("Image with key='" + imageKey + "' not found!");
    }
}

string PppEngine::setInputImage(const cv::Mat & inputImage) const
{
    return m_pImageStore->setImage(inputImage);
}

bool PppEngine::detectLandMarks(const string & imageKey, LandMarks & landMarks) const
{
    verifyImageExists(imageKey);
    // Convert the image to gray scale as needed by some algorithms

    const auto & inputImage = m_pImageStore->getImage(imageKey);
    cv::Mat grayImage;
    cvtColor(inputImage, grayImage, cv::COLOR_BGR2GRAY);

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
        const auto faceRect = Utilities::convert(landMarks.vjFaceRect);
        auto shape = (*m_shapePredictor)(dlibImage, faceRect);

        landMarks.vjFaceRect = Utilities::convert(faceRect);

        landMarks.lipLeftCorner = getLandMark(shape, LandMarkType::LIPS_LEFT_CORNER);
        landMarks.lipRightCorner = getLandMark(shape, LandMarkType::LIPS_RIGHT_CORNER);
        landMarks.eyeLeftPupil = getLandMark(shape, LandMarkType::EYE_LEFT_PUPIL);
        landMarks.eyeRightPupil = getLandMark(shape, LandMarkType::EYE_RIGHT_PUPIL);
        landMarks.chinPoint = getLandMark(shape, LandMarkType::CHIN_LOWEST_POINT);

        for (size_t i = 0; i < shape.num_parts(); ++i)
        {
            landMarks.allLandmarks.push_back(Utilities::convert(shape.part(i)));
        }
    }

    // Estimate chin and crown point (maths from existing landmarks)
    return m_pCrownChinEstimator->estimateCrownChin(landMarks);
}

cv::Point PppEngine::getLandMark(const dlib::full_object_detection & shape, LandMarkType type) const
{
    const auto & indices = m_landmarkIndexMapping.at(type);
    cv::Point result;
    for (const auto & idx : indices)
    {
        result += Utilities::convert(shape.part(idx));
    }
    result = result / static_cast<int>(indices.size());
    return result;
}

cv::Mat PppEngine::cropPicture(const string & imageKey,
                               PhotoStandard & ps,
                               CanvasDefinition & canvas,
                               cv::Point & crownMark,
                               cv::Point & chinMark) const
{
    verifyImageExists(imageKey);
    const auto & inputImage = m_pImageStore->getImage(imageKey);
    return m_pPhotoPrintMaker->cropPicture(inputImage, crownMark, chinMark, ps);
}

cv::Mat PppEngine::createTiledPrint(const string & imageKey,
                                    PhotoStandard & ps,
                                    CanvasDefinition & canvas,
                                    cv::Point & crownMark,
                                    cv::Point & chinMark) const
{

    const auto croppedImage = cropPicture(imageKey, ps, canvas, crownMark, chinMark);

    auto tiledPrintPhoto = m_pPhotoPrintMaker->tileCroppedPhoto(canvas, ps, croppedImage);

    return tiledPrintPhoto;
}

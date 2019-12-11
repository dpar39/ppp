
#include <istream>
#include <streambuf>

#include "ComplianceChecker.h"
#include "ComplianceResult.h"
#include "CrownChinEstimator.h"
#include "EyeDetector.h"
#include "FaceDetector.h"
#include "FileSystem.h"
#include "ImageStore.h"
#include "LandMarks.h"
#include "LipsDetector.h"
#include "PhotoPrintMaker.h"
#include "PhotoStandard.h"
#include "PppEngine.h"
#include "PrintDefinition.h"
#include "Utilities.h"

#include <dlib/image_processing/shape_predictor.h>
#include <dlib/image_transforms/assign_image.h>
#include <dlib/opencv/cv_image.h>
#include <opencv2/imgproc/imgproc.hpp>

using namespace std;

namespace ppp
{

PppEngine::PppEngine(const IDetectorSPtr & pFaceDetector,
                     const IDetectorSPtr & pEyesDetector,
                     const IDetectorSPtr & pLipsDetector,
                     const ICrownChinEstimatorSPtr & pCrownChinEstimator,
                     const IPhotoPrintMakerSPtr & pPhotoPrintMaker,
                     const IImageStoreSPtr & pImageStore,
                     const IComplianceCheckerSPtr & pComplianceChecker)
: m_pFaceDetector(pFaceDetector ? pFaceDetector : make_shared<FaceDetector>())
, m_pEyesDetector(pEyesDetector ? pEyesDetector : make_shared<EyeDetector>())
, m_pLipsDetector(pLipsDetector ? pLipsDetector : make_shared<LipsDetector>())
, m_pCrownChinEstimator(pCrownChinEstimator ? pCrownChinEstimator : make_shared<CrownChinEstimator>())
, m_complianceChecker(pComplianceChecker ? pComplianceChecker : make_shared<ComplianceChecker>())
, m_pPhotoPrintMaker(pPhotoPrintMaker ? pPhotoPrintMaker : make_shared<PhotoPrintMaker>())
, m_pImageStore(pImageStore ? pImageStore : make_shared<ImageStore>())
, m_useDlibLandmarkDetection(false)
{
}

struct Membuf : std::streambuf
{
    Membuf(char const * base, const size_t size)
    {
        char * p(const_cast<char *>(base));
        this->setg(p, p, p + size);
    }
};
struct Imemstream : virtual Membuf, std::istream
{
    Imemstream(char const * base, const size_t size)
    : Membuf(base, size)
    , std::istream(static_cast<std::streambuf *>(this))
    {
    }
};

bool PppEngine::isConfigured() const
{
    return m_shapePredictor != nullptr && m_pFaceDetector->isConfigured() && m_pEyesDetector->isConfigured()
        && m_pLipsDetector->isConfigured();
}

bool PppEngine::configure(const std::string & configFilePathOrContent)
{

    std::ifstream ifs(configFilePathOrContent, std::ios_base::in);
    rapidjson::Document config;
    if (ifs.good())
    {
        // We are on a filesystem, let's get the config's directory
        const auto found = configFilePathOrContent.find_last_of("/\\");
        const auto dir = found != std::string::npos ? configFilePathOrContent.substr(0, found) : "";

        const std::string configString((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
        config.Parse(configString.c_str());
        FileSystem::setPathMapper([dir](const std::string & v) { return dir + "/" + v; });
    }
    else
    {
        config.Parse(configFilePathOrContent);
    }

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
        FileSystem::loadFile(shapePredictorFile, [this, &shapePredictor](const bool success, std::istream & stream) {
            if (success)
            {
                deserialize(*m_shapePredictor, stream);
            }
            else
            {
                const auto shapePredictorFileContent = shapePredictor["data"].GetString();
                const auto shapePredictorFileContentLen = shapePredictor["data"].GetStringLength();
                auto spData = Utilities::base64Decode(shapePredictorFileContent, shapePredictorFileContentLen);
                Imemstream stream(reinterpret_cast<char *>(&spData[0]), spData.size());
                deserialize(*m_shapePredictor, stream);
            }
        });

        // Prepare landmark mapping
        set<int> missingLandMarks;
        auto array = shapePredictor["missingPoints"].GetArray();
        for (rapidjson::SizeType i = 0; i < array.Size(); i++)
        {
            missingLandMarks.insert(array[i].GetInt());
        }

        m_landmarkIndexMapping = { { LandMarkType::EYE_PUPIL_CENTER_LEFT, std::vector<int> { 38, 39, 41, 42 } },
                                   { LandMarkType::EYE_PUPIL_CENTER_RIGHT, std::vector<int> { 44, 45, 47, 48 } },
                                   { LandMarkType::MOUTH_CORNER_LEFT, std::vector<int> { 49 } },
                                   { LandMarkType::MOUTH_CORNER_RIGHT, std::vector<int> { 55 } },
                                   { LandMarkType::CHIN_LOWEST_POINT, std::vector<int> { 9 } },
                                   { LandMarkType::NOSE_TIP_POINT, std::vector<int> { 34 } },
                                   { LandMarkType::EYE_OUTER_CORNER_LEFT, std::vector<int> { 37 } },
                                   { LandMarkType::EYE_OUTER_CORNER_RIGHT, std::vector<int> { 46 } } };

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

bool PppEngine::detectLandMarks(const string & imageKey) const
{
    verifyImageExists(imageKey);
    // Convert the image to gray scale as needed by some algorithms

    const auto & inputImage = m_pImageStore->getImage(imageKey);
    const auto & landMarks = m_pImageStore->getLandMarks(imageKey);
    cv::Mat grayImage;
    cvtColor(inputImage, grayImage, cv::COLOR_BGR2GRAY);

    // Detect the face
    if (!m_pFaceDetector->detectLandMarks(grayImage, *landMarks))
    {
        return false;
    }

    if (!m_useDlibLandmarkDetection)
    {
        // Detect the eye pupils
        if (!m_pEyesDetector->detectLandMarks(grayImage, *landMarks))
        {
            return false;
        }

        // Detect mouth landmarks
        if (!m_pLipsDetector->detectLandMarks(inputImage, *landMarks))
        {
            return false;
        }
    }
    else
    {
        using namespace dlib;
        // Detect the face
        if (!m_pFaceDetector->detectLandMarks(grayImage, *landMarks))
        {
            return false;
        }
        array2d<bgr_pixel> dlibImage;
        assign_image(dlibImage, cv_image<bgr_pixel>(inputImage));

        const auto & r = landMarks->vjFaceRect;
        const auto faceRect = rectangle(r.x, r.y, r.x + r.width, r.y + r.height);
        auto shape = (*m_shapePredictor)(dlibImage, faceRect);

        const auto numParts = shape.num_parts();
        landMarks->allLandmarks.clear();
        landMarks->allLandmarks.reserve(numParts);
        for (size_t i = 0; i < numParts; ++i)
        {
            auto & part = shape.part(i);
            landMarks->allLandmarks.emplace_back(part.x(), part.y());
        }

        const auto & lms = landMarks->allLandmarks;
        landMarks->lipLeftCorner = getLandMark(lms, LandMarkType::MOUTH_CORNER_LEFT);
        landMarks->lipRightCorner = getLandMark(lms, LandMarkType::MOUTH_CORNER_RIGHT);
        landMarks->eyeLeftPupil = getLandMark(lms, LandMarkType::EYE_PUPIL_CENTER_LEFT);
        landMarks->eyeRightPupil = getLandMark(lms, LandMarkType::EYE_PUPIL_CENTER_RIGHT);
        landMarks->chinPoint = getLandMark(lms, LandMarkType::CHIN_LOWEST_POINT);
        landMarks->noseTip = getLandMark(lms, LandMarkType::NOSE_TIP_POINT);
        landMarks->eyeLeftCorner = getLandMark(lms, LandMarkType::EYE_OUTER_CORNER_LEFT);
        landMarks->eyeRightCorner = getLandMark(lms, LandMarkType::EYE_OUTER_CORNER_RIGHT);
    }

    // Estimate chin and crown point (maths from existing landmarks)
    return m_pCrownChinEstimator->estimateCrownChin(*landMarks);
}

cv::Point PppEngine::getLandMark(const std::vector<cv::Point> & landmarks, const LandMarkType type) const
{
    const auto & indices = m_landmarkIndexMapping.at(type);
    if (indices.size() == 1)
    {
        return landmarks[indices.front()];
    }

    cv::Point result;
    for (const auto & idx : indices)
    {
        result += landmarks[idx];
    }
    result = result / static_cast<int>(indices.size());
    return result;
}

cv::Mat PppEngine::createTiledPrint(const string & imageKey,
                                    PhotoStandard & ps,
                                    PrintDefinition & pd,
                                    cv::Point & crownMark,
                                    cv::Point & chinMark) const
{
    verifyImageExists(imageKey);
    const auto & inputImage = m_pImageStore->getImage(imageKey);
    const auto croppedImage = m_pPhotoPrintMaker->cropPicture(inputImage, crownMark, chinMark, ps);
    auto tiledPrintPhoto = m_pPhotoPrintMaker->tileCroppedPhoto(pd, ps, croppedImage);
    return tiledPrintPhoto;
}

IImageStoreSPtr PppEngine::getImageStore() const
{
    return m_pImageStore;
}

std::string PppEngine::checkCompliance(const std::string & imageId,
                                       const PhotoStandardSPtr & photoStandard,
                                       const cv::Point & crownPoint,
                                       const cv::Point & chinPoint,
                                       const std::vector<std::string> & complianceCheckNames) const
{

    const auto results
        = m_complianceChecker->checkCompliance(imageId, photoStandard, crownPoint, chinPoint, complianceCheckNames);
    rapidjson::Document d;
    d.SetArray();
    auto & alloc = d.GetAllocator();
    for (const auto & result : results)
    {
        d.PushBack(result->toJson(alloc).Move(), alloc);
    }
    return Utilities::serializeJson(d, false);
}
} // namespace ppp

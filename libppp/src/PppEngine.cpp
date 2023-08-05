#include "PppEngine.h"
#include "ComplianceChecker.h"
#include "ComplianceResult.h"
#include "ConfigLoader.h"
#include "CrownChinEstimator.h"
#include "FaceMeshExtractor.h"
#include "ImageStore.h"
#include "LandMarks.h"
#include "PhotoPrintMaker.h"
#include "PhotoStandard.h"
#include "PrintDefinition.h"
#include "Utilities.h"

#include <glog/logging.h>
#include <opencv2/imgproc/imgproc.hpp>

#include <istream>

using namespace std;

namespace ppp {

PppEngine::PppEngine(const IFaceMeshExtractorSPtr & pFaceMeshExtractor,
                     const ICrownChinEstimatorSPtr & pCrownChinEstimator,
                     const IPhotoPrintMakerSPtr & pPhotoPrintMaker,
                     const IImageStoreSPtr & pImageStore,
                     const IComplianceCheckerSPtr & pComplianceChecker)
: m_pFaceMeshExtractor(pFaceMeshExtractor ? pFaceMeshExtractor : make_shared<FaceMeshExtractor>())
, m_pCrownChinEstimator(pCrownChinEstimator ? pCrownChinEstimator : make_shared<CrownChinEstimator>())
, m_complianceChecker(pComplianceChecker ? pComplianceChecker : make_shared<ComplianceChecker>())
, m_pPhotoPrintMaker(pPhotoPrintMaker ? pPhotoPrintMaker : make_shared<PhotoPrintMaker>())
, m_pImageStore(pImageStore ? pImageStore : make_shared<ImageStore>())
{
}

bool PppEngine::isConfigured() const
{
    return m_pFaceMeshExtractor->isConfigured();
}

bool PppEngine::configure(const std::string & configFilePathOrContent)
{
    const auto configLoader = std::make_shared<ConfigLoader>(configFilePathOrContent);
    m_pFaceMeshExtractor->configure(configLoader);
    m_pCrownChinEstimator->configure(configLoader);
    m_pImageStore->configure(configLoader);
    m_pPhotoPrintMaker->configure(configLoader);
    return true;
}

void PppEngine::verifyImageExists(const string & imageKey) const
{
    if (!m_pImageStore->containsImage(imageKey)) {
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
    if (!m_pFaceMeshExtractor->detectLandMarks(inputImage, *landMarks)) {
        return false;
    }
    // Estimate chin and crown point (maths from existing landmarks)
    return m_pCrownChinEstimator->estimateCrownChin(*landMarks);
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
    for (const auto & result : results) {
        d.PushBack(result->toJson(alloc).Move(), alloc);
    }
    return Utilities::serializeJson(d, false);
}
} // namespace ppp

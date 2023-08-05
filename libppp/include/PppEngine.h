#pragma once

#include "CommonHelpers.h"
#include "IFaceMeshExtractor.h"
#include "PhotoStandard.h"

#include <opencv2/core/core.hpp>

#include <unordered_map>

namespace dlib {
class full_object_detection;
class shape_predictor;
} // namespace dlib

namespace ppp {
FWD_DECL(LandMarks)
FWD_DECL(IFaceMeshExtractor)
FWD_DECL(ICrownChinEstimator)
FWD_DECL(IImageStore)
FWD_DECL(IPhotoPrintMaker)
FWD_DECL(IComplianceChecker)

class PrintDefinition;
class PhotoStandard;

FWD_DECL(PppEngine)

struct EnumClassHash
{
    template <typename T>
    std::size_t operator()(T t) const
    {
        return static_cast<std::size_t>(t);
    }
};

class PppEngine final : NonCopyable
{
public:
    explicit PppEngine(const IFaceMeshExtractorSPtr & pFaceMeshExtractor = nullptr,
                       const ICrownChinEstimatorSPtr & pCrownChinEstimator = nullptr,
                       const IPhotoPrintMakerSPtr & pPhotoPrintMaker = nullptr,
                       const IImageStoreSPtr & pImageStore = nullptr,
                       const IComplianceCheckerSPtr & pComplianceChecker = nullptr);

    bool isConfigured() const;
    // Native interface
    bool configure(const std::string & configFilePathOrContent);

    bool detectLandMarks(const std::string & imageKey) const;

    cv::Mat createTiledPrint(const std::string & imageKey,
                             PhotoStandard & ps,
                             PrintDefinition & pd,
                             cv::Point & crownMark,
                             cv::Point & chinMark) const;

    IImageStoreSPtr getImageStore() const;
    std::string checkCompliance(const std::string & imageId,
                                const PhotoStandardSPtr & photoStandard,
                                const cv::Point & crownPoint,
                                const cv::Point & chinPoint,
                                const std::vector<std::string> & complianceCheckNames) const;

private:
    IFaceMeshExtractorSPtr m_pFaceMeshExtractor;
    ICrownChinEstimatorSPtr m_pCrownChinEstimator;
    IComplianceCheckerSPtr m_complianceChecker;

    IPhotoPrintMakerSPtr m_pPhotoPrintMaker;
    IImageStoreSPtr m_pImageStore;

    void verifyImageExists(const std::string & imageKey) const;
};
} // namespace ppp

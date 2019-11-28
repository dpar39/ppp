#pragma once
#include "CommonHelpers.h"
#include <opencv2/core/types.hpp>
#include <rapidjson/document.h>

namespace ppp
{
FWD_DECL(IComplianceChecker)
FWD_DECL(PhotoStandard)

class IComplianceChecker : NonCopyable
{
public:
    virtual ~IComplianceChecker() = default;

    /*!@brief Configure general parameters for compliance checks:  !*/
    virtual void configure(rapidjson::Value & cfg) = 0;

    virtual std::string checkCompliance(const std::string & imgKey,
                                        const PhotoStandardSPtr & photoStandard,
                                        const cv::Point & crownPoint,
                                        const cv::Point & chinPoint,
                                        const std::vector<std::string> & complianceCheckNames)
        = 0;
};
} // namespace ppp

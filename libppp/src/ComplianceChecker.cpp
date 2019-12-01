
#include "ComplianceChecker.h"
#include "PhotoStandard.h"

namespace ppp
{

void ComplianceChecker::configure(rapidjson::Value & cfg)
{
}

std::string ComplianceChecker::checkCompliance(const std::string & imgKey,
                                               const PhotoStandardSPtr & photoStandard,
                                               const cv::Point & crownPoint,
                                               const cv::Point & chinPoint,
                                               const std::vector<std::string> & complianceCheckNames)
{
    for (const auto & checkName : complianceCheckNames)
    {
        if (checkName == "inputResolution")
        {
            photoStandard->faceHeightMM();
            const auto distPix = norm(crownPoint - chinPoint);
            
        }
    }
    return "";
}
} // namespace ppp

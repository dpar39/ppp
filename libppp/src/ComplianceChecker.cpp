
#include "ComplianceChecker.h"

namespace ppp
{

void ComplianceChecker::configure(rapidjson::Value & cfg)
{
}

std::string ComplianceChecker::checkCompliance(const PhotoStandardSPtr & photoStandard,
                                               const cv::Point & crownPoint,
                                               const cv::Point & chinPoint,
                                               const std::vector<std::string> & complianceCheckNames)
{
    return "";
}
} // namespace ppp

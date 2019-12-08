#pragma once

#include "IComplianceChecker.h"

DEFINE_STR(CHECK_INPUT_RESOLUTION, inpuResolution)

namespace ppp
{
FWD_DECL(ComplianceChecker);
FWD_DECL(ComplianceResult);

class ComplianceChecker final : public IComplianceChecker
{
public:
    ~ComplianceChecker() override = default;

    void configure(rapidjson::Value & cfg) override;

    std::vector<ComplianceResultSPtr> checkCompliance(const std::string & imgKey,
                                                      const PhotoStandardSPtr & photoStandard,
                                                      const cv::Point & crownPoint,
                                                      const cv::Point & chinPoint,
                                                      const std::vector<std::string> & complianceCheckNames) override;

private:
    ComplianceResultSPtr checkInputResolution(const std::string & imgKey,
                                              const PhotoStandardSPtr & photoStandard,
                                              const cv::Point & crownPoint,
                                              const cv::Point & chinPoint) const;
};
} // namespace ppp

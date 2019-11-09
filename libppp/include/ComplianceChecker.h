#pragma once

#include "IComplianceChecker.h"

namespace ppp
{
FWD_DECL(ComplianceChecker);

class ComplianceChecker final : public IComplianceChecker
{
public:
    ~ComplianceChecker() = default;

    void configure(rapidjson::Value & cfg) override;

    std::string checkCompliance(const PhotoStandardSPtr & photoStandard,
                                const cv::Point & crownPoint,
                                const cv::Point & chinPoint,
                                const std::vector<std::string> & complianceCheckNames) override;
};
} // namespace ppp

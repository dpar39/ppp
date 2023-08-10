#pragma once

#include "IComplianceChecker.h"

#include <opencv2/core/core.hpp>

DEFINE_STR(CHECK_INPUT_RESOLUTION, inpuResolution)

namespace ppp {
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

    ComplianceResultSPtr checkBlurriness(const std::string & imgKey,
                                         const PhotoStandardSPtr & photoStandard,
                                         const cv::Point & crownPoint,
                                         const cv::Point & chinPoint) const;
    /// ICAO requirements
    // Blurred
    // Looking Away
    // Ink marks/creased
    // Unnatural skin tones
    // Too dark/light
    // Washed out
    // Pixelation
    // Hair accross eyes
    // Eyes closed
    // Varied background
    // Yaw/pitch/roll greather than threshold
    // Flash reflection on skin
    // Red eyes
    // Shadows behind head
    // Shadows accross face
    // Dark tinted lenses
    // Flash reflection on lenses
    // Glass frames too heavy
    // Glass frames covering eyes
    // Hat/Cap
    // Veil over face
    // Mouth open
    // Presence of other faces or toys too close to face
    // Photo not older than 6 months

private:
    cv::Mat m_psCropped;
};
} // namespace ppp

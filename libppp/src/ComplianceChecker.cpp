
#include "ComplianceChecker.h"
#include "ComplianceResult.h"
#include "PhotoStandard.h"
#include "Utilities.h"

namespace ppp
{

void ComplianceChecker::configure(rapidjson::Value & cfg)
{
}

std::vector<ComplianceResultSPtr> ComplianceChecker::checkCompliance(
    const std::string & imgKey,
    const PhotoStandardSPtr & photoStandard,
    const cv::Point & crownPoint,
    const cv::Point & chinPoint,
    const std::vector<std::string> & complianceCheckNames)
{
    std::vector<ComplianceResultSPtr> results;
    using namespace std;
    using namespace rapidjson;
    for (const auto & checkName : complianceCheckNames)
    {
        if (checkName == CHECK_INPUT_RESOLUTION)
        {
            const auto result = checkInputResolution(imgKey, photoStandard, crownPoint, chinPoint);
            results.push_back(result);
        }
    }

    return results;
}

ComplianceResultSPtr ComplianceChecker::checkInputResolution(const std::string & imgKey,
                                                             const PhotoStandardSPtr & photoStandard,
                                                             const cv::Point & crownPoint,
                                                             const cv::Point & chinPoint) const
{
    using namespace std;
    const auto distPix = norm(crownPoint - chinPoint);
    const auto distInch = photoStandard->faceHeight("inch");
    const auto inputDpi = distPix / distInch;

    const auto minDpiFromPhotoStandard = 1.25; // input
    const auto success = inputDpi > minDpiFromPhotoStandard * photoStandard->resolutionDpi();
    string message;
    if (success)
    {
        message = "Image's input resolution is greater than the specified in the photo standard.";
    }
    else
    {
        message = "Image's input resolution is low. Try taking a photo closer or instead use a better camera.";
    }
    const auto result = std::make_shared<ComplianceResult>(CHECK_INPUT_RESOLUTION, success, message);
    result->setParam(CHECK_INPUT_RESOLUTION, inputDpi);
    return result;
}

} // namespace ppp

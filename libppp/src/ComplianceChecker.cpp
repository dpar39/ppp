
#include "ComplianceChecker.h"
#include "ComplianceResult.h"
#include "PhotoStandard.h"
#include "Utilities.h"

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
    std::vector<ComplianceResultSPtr> results;
    using namespace std;
    using namespace rapidjson;
    for (const auto & checkName : complianceCheckNames)
    {
        if (checkName == "inputResolution")
        {
            const auto distPix = norm(crownPoint - chinPoint);
            const auto distInch = photoStandard->faceHeight("inch");
            const auto inputDpi = distPix / distInch;

            const auto minDpiFromPhotoStandard = 1.5; // input
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
            const auto result = make_shared<ComplianceResult>(checkName, success, message);
            results.push_back(result);
        }
    }

    Document d;
    d.SetArray();
    auto & alloc = d.GetAllocator();
    for (const auto & result : results)
    {
        d.PushBack(result->toJson(alloc).Move(), alloc);
    }
    return Utilities::serializeJson(d, false);
}
} // namespace ppp

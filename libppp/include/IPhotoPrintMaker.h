#pragma once

#include "CommonHelpers.h"
#include <opencv2/core/core.hpp>
#include <rapidjson/document.h>

namespace ppp
{
class PrintDefinition;
class PhotoStandard;

FWD_DECL(IPhotoPrintMaker)

class IPhotoPrintMaker : NonCopyable
{
public:
    virtual ~IPhotoPrintMaker() = default;

    virtual void configure(rapidjson::Value & cfg) = 0;

    virtual cv::Mat cropPicture(const cv::Mat & originalImage,
                                const cv::Point & crownPoint,
                                const cv::Point & chinPoint,
                                const PhotoStandard & ps)
        = 0;

    virtual cv::Mat tileCroppedPhoto(const PrintDefinition & pd, const PhotoStandard & ps, const cv::Mat & croppedImage)
        = 0;
};
} // namespace ppp

#pragma once

#include "CommonHelpers.h"
#include "IConfigurable.h"
#include <opencv2/core/core.hpp>

namespace ppp {
FWD_DECL(PrintDefinition)
FWD_DECL(PhotoStandard)
FWD_DECL(ConfigLoader)

FWD_DECL(IPhotoPrintMaker)

class IPhotoPrintMaker : NonCopyable, public IConfigurable
{
public:
    virtual ~IPhotoPrintMaker() = default;

    virtual cv::Mat cropPicture(const cv::Mat & originalImage,
                                const cv::Point & crownPoint,
                                const cv::Point & chinPoint,
                                const PhotoStandard & ps)
        = 0;

    virtual cv::Mat tileCroppedPhoto(const PrintDefinition & pd, const PhotoStandard & ps, const cv::Mat & croppedImage)
        = 0;
};
} // namespace ppp

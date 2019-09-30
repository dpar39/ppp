#pragma once

#include "CommonHelpers.h"
#include "IPhotoPrintMaker.h"

#include <opencv2/core/core.hpp>

FWD_DECL(PhotoPrintMaker)
class CanvasDefinition;
class PhotoStandard;

class PhotoPrintMaker final : public IPhotoPrintMaker
{
public:
    void configure(rapidjson::Value & cfg) override;

    cv::Mat cropPicture(const cv::Mat & originalImage,
                        const cv::Point & crownPoint,
                        const cv::Point & chinPoint,
                        const PhotoStandard & ps) override;

    // Creates a tiled photo from the cropped photo
    cv::Mat tileCroppedPhoto(const CanvasDefinition & canvas,
                             const PhotoStandard & ps,
                             const cv::Mat & croppedImage) override;

private:
    cv::Point2d centerCropEstimation(const PhotoStandard & ps,
                                     const cv::Point & crownPoint,
                                     const cv::Point & chinPoint) const;

    cv::Scalar m_backgroundColor = cv::Scalar(128, 128, 128);
};

;

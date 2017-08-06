#include "PhotoStandard.h"
#include "CanvasDefinition.h"
#include "PhotoPrintMaker.h"

#include "Utilities.h"

#include <opencv2/imgproc/imgproc.hpp>

using namespace cv;

void PhotoPrintMaker::configure(rapidjson::Value& cfg)
{
    auto &ppmConfig = cfg["photoPrintMaker"];
    auto rgbArr = ppmConfig["background"].GetArray();
    m_backgroundColor = Scalar(rgbArr[0].GetInt(), rgbArr[1].GetInt(), rgbArr[2].GetInt());
}

cv::Mat PhotoPrintMaker::cropPicture(const cv::Mat& originalImage,
                                     const cv::Point& crownPoint,
                                     const cv::Point& chinPoint,
                                     const PhotoStandard& ps)
{

    auto centerCrop = centerCropEstimation(ps, crownPoint, chinPoint);


    auto chinCrownVec = crownPoint - chinPoint;

    auto faceHeightPix = cv::norm(chinCrownVec);

    auto cropHeightPix = ps.photoHeightMM() / ps.faceHeightMM() * faceHeightPix;
    auto cropWidthPix = ps.photoWidthMM() / ps.photoHeightMM() * cropHeightPix;

    auto centerTop = centerCrop + cv::Point2d(chinCrownVec*(cropHeightPix / faceHeightPix/ 2.0));

    auto chinCrown90degRotated = Point2d(chinCrownVec.y, -chinCrownVec.x);
    auto centerLeft = centerCrop + chinCrown90degRotated*(cropWidthPix / faceHeightPix / 2.0);

    const Point2f srcs[3] = {centerCrop, centerLeft, centerTop};
    const Point2f dsts[3] = {Point2d(cropWidthPix / 2.0, cropHeightPix / 2.0), Point2d(0.0, cropHeightPix / 2.0), Point2d(cropWidthPix / 2.0, 0.0)};
    auto tform = getAffineTransform(srcs, dsts);

    Mat cropImage;
    warpAffine(originalImage, cropImage, tform, cv::Size(ROUND_INT(cropWidthPix), ROUND_INT(cropHeightPix)));
    return cropImage;
}

cv::Mat PhotoPrintMaker::tileCroppedPhoto(const CanvasDefinition& canvas, const PhotoStandard& ps, const cv::Mat& croppedImage)
{
    auto canvasWidthPixels = CEIL_INT(canvas.resolutionPixelsPerMM()*canvas.width());
    auto canvasHeightPixels = CEIL_INT(canvas.resolutionPixelsPerMM()*canvas.height());

    auto numPhotoRows = static_cast<size_t>(canvas.height() / (ps.photoHeightMM() + canvas.border()));
    auto numPhotoCols = static_cast<size_t>(canvas.width() / (ps.photoWidthMM() + canvas.border()));

    cv::Size tileSizePixels(ROUND_INT(canvas.resolutionPixelsPerMM() * ps.photoWidthMM()), ROUND_INT(canvas.resolutionPixelsPerMM() * ps.photoHeightMM()));

    // Resize input crop to the canvas output
    cv::Mat tileInCanvas;
    cv::resize(croppedImage, tileInCanvas, tileSizePixels);

    cv::Mat printPhoto(canvasHeightPixels, canvasWidthPixels, croppedImage.type(), m_backgroundColor);

    auto dx = ROUND_INT((ps.photoWidthMM() + canvas.border())*canvas.resolutionPixelsPerMM());
    auto dy = ROUND_INT((ps.photoHeightMM() + canvas.border())*canvas.resolutionPixelsPerMM());
    for (size_t row = 0; row < numPhotoRows; ++row)
    {
        for (size_t col = 0; col < numPhotoCols; ++col)
        {
            cv::Point topLeft(static_cast<int>(col)*dx, static_cast<int>(row)*dy);
            tileInCanvas.copyTo(printPhoto(cv::Rect(topLeft, tileSizePixels)));
        }
    }
    return printPhoto;
}

cv::Point2d PhotoPrintMaker::centerCropEstimation(const PhotoStandard& ps, const cv::Point& crownPoint, const cv::Point& chinPoint) const
{
    if (ps.eyesHeightMM() <= 0)
    {
        // Estimate the center of the picture to be the median point between the crown point and the chin point
        return (crownPoint + chinPoint) / 2.0;
    }

    const auto eyeCrownToFaceHeightRatio = 0.5;

    auto crownToPictureBottomMM = eyeCrownToFaceHeightRatio * ps.faceHeightMM() + ps.eyesHeightMM();

    auto crownToCenterMM = crownToPictureBottomMM - ps.photoHeightMM() / 2;

    auto mmToPixRatio = cv::norm(crownPoint - chinPoint)/ps.faceHeightMM();

    auto crownToCenterPix = mmToPixRatio*crownToCenterMM;

    return pointInLineAtDistance(crownPoint, chinPoint, crownToCenterPix);
}

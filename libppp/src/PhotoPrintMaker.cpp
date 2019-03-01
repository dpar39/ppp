#include "PhotoPrintMaker.h"
#include "CanvasDefinition.h"
#include "PhotoStandard.h"

#include "Utilities.h"

#include <opencv2/imgproc/imgproc.hpp>

using namespace cv;

void PhotoPrintMaker::configure(rapidjson::Value & cfg)
{
    auto & ppmConfig = cfg["photoPrintMaker"];
    const auto rgbArr = ppmConfig["background"].GetArray();
    m_backgroundColor = Scalar(rgbArr[0].GetInt(), rgbArr[1].GetInt(), rgbArr[2].GetInt());
}

Mat PhotoPrintMaker::cropPicture(const Mat & originalImage,
                                 const Point & crownPoint,
                                 const Point & chinPoint,
                                 const PhotoStandard & ps)
{
    const auto centerCrop = centerCropEstimation(ps, crownPoint, chinPoint);

    const auto chinCrownVec = crownPoint - chinPoint;

    const auto faceHeightPix = norm(chinCrownVec);

    const auto cropHeightPix = ps.photoHeightMM() / ps.faceHeightMM() * faceHeightPix;
    const auto cropWidthPix = ps.photoWidthMM() / ps.photoHeightMM() * cropHeightPix;

    const auto centerTop = centerCrop + Point2d(chinCrownVec * (cropHeightPix / faceHeightPix / 2.0));

    const auto chinCrown90degRotated = Point2d(chinCrownVec.y, -chinCrownVec.x);
    const auto centerLeft = centerCrop + chinCrown90degRotated * (cropWidthPix / faceHeightPix / 2.0);

    const Point2f srcs[3] = { centerCrop, centerLeft, centerTop };
    const Point2f dsts[3] = { Point2d(cropWidthPix / 2.0, cropHeightPix / 2.0),
                              Point2d(0.0, cropHeightPix / 2.0),
                              Point2d(cropWidthPix / 2.0, 0.0) };
    const auto tform = getAffineTransform(srcs, dsts);

    Mat cropImage;
    warpAffine(originalImage, cropImage, tform, Size(ROUND_INT(cropWidthPix), ROUND_INT(cropHeightPix)));
    return cropImage;
}

Mat PhotoPrintMaker::tileCroppedPhoto(const CanvasDefinition & canvas,
                                      const PhotoStandard & ps,
                                      const Mat & croppedImage)
{
    const auto canvasWidthPixels = CEIL_INT(canvas.resolutionPixelsPerMM() * canvas.width());
    const auto canvasHeightPixels = CEIL_INT(canvas.resolutionPixelsPerMM() * canvas.height());

    const auto numPhotoRows = static_cast<size_t>(canvas.height() / (ps.photoHeightMM() + canvas.border()));
    const auto numPhotoCols = static_cast<size_t>(canvas.width() / (ps.photoWidthMM() + canvas.border()));

    const Size tileSizePixels(ROUND_INT(canvas.resolutionPixelsPerMM() * ps.photoWidthMM()),
                              ROUND_INT(canvas.resolutionPixelsPerMM() * ps.photoHeightMM()));

    // Resize input crop to the canvas output
    Mat tileInCanvas;
    resize(croppedImage, tileInCanvas, tileSizePixels);

    Mat printPhoto(canvasHeightPixels, canvasWidthPixels, croppedImage.type(), m_backgroundColor);

    const auto dx = ROUND_INT((ps.photoWidthMM() + canvas.border()) * canvas.resolutionPixelsPerMM());
    const auto dy = ROUND_INT((ps.photoHeightMM() + canvas.border()) * canvas.resolutionPixelsPerMM());
    for (size_t row = 0; row < numPhotoRows; ++row)
    {
        for (size_t col = 0; col < numPhotoCols; ++col)
        {
            Point topLeft(static_cast<int>(col) * dx, static_cast<int>(row) * dy);
            tileInCanvas.copyTo(printPhoto(Rect(topLeft, tileSizePixels)));
        }
    }
    return printPhoto;
}

Point2d PhotoPrintMaker::centerCropEstimation(const PhotoStandard & ps,
                                              const Point & crownPoint,
                                              const Point & chinPoint) const
{
    if (ps.eyesHeightMM() <= 0)
    {
        // Estimate the center of the picture to be the median point between the crown point and the chin point
        return (crownPoint + chinPoint) / 2.0;
    }

    const auto eyeCrownToFaceHeightRatio = 0.5;

    const auto crownToPictureBottomMM = eyeCrownToFaceHeightRatio * ps.faceHeightMM() + ps.eyesHeightMM();

    const auto crownToCenterMM = crownToPictureBottomMM - ps.photoHeightMM() / 2;

    const auto mmToPixRatio = norm(crownPoint - chinPoint) / ps.faceHeightMM();

    const auto crownToCenterPix = mmToPixRatio * crownToCenterMM;

    return Utilities::pointInLineAtDistance(crownPoint, chinPoint, crownToCenterPix);
}

#include "PhotoPrintMaker.h"
#include "CanvasDefinition.h"
#include "PhotoStandard.h"
#include "Utilities.h"

#include <opencv2/imgproc/imgproc.hpp>

using namespace cv;

namespace ppp
{

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
    warpAffine(originalImage, cropImage, tform, Size(roundInteger(cropWidthPix), roundInteger(cropHeightPix)));
    return cropImage;
}

Mat PhotoPrintMaker::tileCroppedPhoto(const CanvasDefinition & canvas,
                                      const PhotoStandard & ps,
                                      const Mat & croppedImage)
{
    const auto canvasWidthPixels = canvas.widthPixels();
    const auto canvasHeightPixels = canvas.heightPixels();
    const auto canvasPaddingPixels = canvas.paddingPixels();

    const auto numPhotoRows = floorInteger(canvas.height() / (ps.photoHeightMM() + canvas.border()));
    const auto numPhotoCols = floorInteger(canvas.width() / (ps.photoWidthMM() + canvas.border()));

    const Size tileSizePixels(roundInteger(canvas.resolutionPixPerMM() * ps.photoWidthMM()),
                              roundInteger(canvas.resolutionPixPerMM() * ps.photoHeightMM()));

    // Resize input crop to the canvas output
    Mat tileInCanvas;
    resize(croppedImage, tileInCanvas, tileSizePixels);

    Mat printPhoto(canvasHeightPixels, canvasWidthPixels, croppedImage.type(), m_backgroundColor);

    const auto dx = roundInteger((ps.photoWidthMM() + canvas.border()) * canvas.resolutionPixPerMM());
    const auto dy = roundInteger((ps.photoHeightMM() + canvas.border()) * canvas.resolutionPixPerMM());

    const auto xOffset = canvasPaddingPixels;
    const auto yOffset = canvasPaddingPixels;

    for (auto row = 0; row < numPhotoRows; ++row)
    {
        for (auto col = 0; col < numPhotoCols; ++col)
        {
            Point topLeft(xOffset + col * dx, yOffset + row * dy);
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
} // namespace ppp

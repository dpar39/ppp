#include "PhotoPrintMaker.h"
#include "ConfigLoader.h"
#include "PhotoStandard.h"
#include "PrintDefinition.h"
#include "Utilities.h"

#include <opencv2/imgproc/imgproc.hpp>

using namespace cv;

namespace ppp
{

void PhotoPrintMaker::configureInternal(const ConfigLoaderSPtr & cfg)
{
    auto & ppmConfig = cfg->get({ "photoPrintMaker" });
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

    const auto cropHeightPix = ps.photoHeight() / ps.faceHeight() * faceHeightPix;
    const auto cropWidthPix = ps.photoWidth() / ps.photoHeight() * cropHeightPix;

    const auto centerTop = centerCrop + Point2d(chinCrownVec * (cropHeightPix / faceHeightPix / 2.0));
    const auto chinCrown90DegRotated = Point2d(chinCrownVec.y, -chinCrownVec.x);
    const auto centerLeft = centerCrop + chinCrown90DegRotated * (cropWidthPix / faceHeightPix / 2.0);

    const Point2f srcPoints[3] = { centerCrop, centerLeft, centerTop };
    const Point2f dstPoints[3] = { Point2d(cropWidthPix / 2.0, cropHeightPix / 2.0),
                                   Point2d(0.0, cropHeightPix / 2.0),
                                   Point2d(cropWidthPix / 2.0, 0.0) };
    const auto transform = getAffineTransform(srcPoints, dstPoints);

    Mat cropImage;
    warpAffine(originalImage, cropImage, transform, Size(roundInteger(cropWidthPix), roundInteger(cropHeightPix)));
    return cropImage;
}

Mat PhotoPrintMaker::tileCroppedPhoto(const PrintDefinition & pd, const PhotoStandard & ps, const Mat & croppedImage)
{
    if (ps.resolutionDpi() > pd.resolutionDpi())
    {
        pd.overrideResolution(ps.resolutionDpi());
    }
    else if (ps.resolutionDpi() < pd.resolutionDpi())
    {
        ps.overrideResolution(pd.resolutionDpi());
    }

    const Size tileSizePixels(roundInteger(ps.photoWidth()), roundInteger(ps.photoHeight()));
    // Resize input crop to the print resolution
    Mat templateImage;
    resize(croppedImage, templateImage, tileSizePixels);
    if (pd.width() <= 0 || pd.height() <= 0)
    {
        // This is digital size output
        return templateImage;
    }

    const auto canvasWidthPixels = roundInteger(pd.totalWidth());
    const auto canvasHeightPixels = roundInteger(pd.totalHeight());
    const auto canvasPaddingPixels = pd.padding();

    const auto numPhotoRows = floorInteger(pd.height() / (ps.photoHeight() + pd.gutter()));
    const auto numPhotoCols = floorInteger(pd.width() / (ps.photoWidth() + pd.gutter()));

    Mat printPhoto(canvasHeightPixels, canvasWidthPixels, croppedImage.type(), m_backgroundColor);

    const auto dx = roundInteger(ps.photoWidth() + pd.gutter());
    const auto dy = roundInteger(ps.photoHeight() + pd.gutter());

    const auto xOffset = canvasPaddingPixels;
    const auto yOffset = canvasPaddingPixels;

    for (auto row = 0; row < numPhotoRows; ++row)
    {
        for (auto col = 0; col < numPhotoCols; ++col)
        {
            Point topLeft(xOffset + col * dx, yOffset + row * dy);
            templateImage.copyTo(printPhoto(Rect(topLeft, tileSizePixels)));
        }
    }
    return printPhoto;
}

Point2d PhotoPrintMaker::centerCropEstimation(const PhotoStandard & ps,
                                              const Point & crownPoint,
                                              const Point & chinPoint) const
{
    if (ps.crownTop() <= 0)
    {
        // Estimate the center of the picture to be the median point between the crown point and the chin point
        return (crownPoint + chinPoint) / 2.0;
    }

    const auto pixelsPerUnits = norm(crownPoint - chinPoint) / ps.faceHeight();
    const auto crownTopPix = ps.crownTop() * pixelsPerUnits;
    const auto picHeightPix = ps.photoHeight() * pixelsPerUnits;
    const auto crownToCenterPix = picHeightPix / 2.0 - crownTopPix;
    return Utilities::pointInLineAtDistance(crownPoint, chinPoint, crownToCenterPix);
}
} // namespace ppp

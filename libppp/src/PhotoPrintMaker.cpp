#include "PhotoStandard.h"
#include "CanvasDefinition.h"
#include "PhotoPrintMaker.h"

#include "Geometry.h"

#include <opencv2/imgproc/imgproc.hpp>

using namespace cv;

cv::Mat PhotoPrintMaker::cropPicture(const cv::Mat& originalImage,
                                     const cv::Point& crownPoint,
                                     const cv::Point& chinPoint,
                                     const PhotoStandard& ps)
{
    auto centerCrop = CENTER_POINT(crownPoint, chinPoint);
    auto faceHeightPix = cv::norm(crownPoint - chinPoint);

    auto cropHeightPix = ps.photoHeightMM() / ps.faceHeightMM() * faceHeightPix;
    auto cropWidthPix = ps.photoWidthMM() / ps.photoHeightMM() * cropHeightPix;

    auto pair1 = pointsAtDistanceParallelToCentreOf(crownPoint, chinPoint, cropHeightPix / 2.0);
    auto centerTop = cv::norm(POINT2D(crownPoint) - pair1.first) 
       < cv::norm(POINT2D(crownPoint) - pair1.second) 
       ? pair1.first : pair1.second;
    auto pair2 = pointsAtDistanceNormalToCentreOf(crownPoint, chinPoint, cropWidthPix / 2.0);
    auto centerLeft = cv::norm(POINT2D(crownPoint) - pair2.first) 
       < cv::norm(POINT2D(crownPoint) - pair2.second) ? pair2.first : pair2.second;

    const Point2f srcs[3] = {centerCrop, centerLeft, centerTop};
    const Point2f dsts[3] = {Point2d(cropWidthPix / 2.0, cropHeightPix / 2.0), Point2d(0.0, cropHeightPix / 2.0), Point2d(cropWidthPix / 2.0, 0.0)};
    Mat tform = getAffineTransform(srcs, dsts);

    Mat cropImage;
    warpAffine(originalImage, cropImage, tform, cv::Size(ROUND_INT(cropWidthPix), ROUND_INT(cropHeightPix)));
    return cropImage;
}

cv::Mat PhotoPrintMaker::tileCroppedPhoto(const CanvasDefinition& canvas, const PhotoStandard& ps, const cv::Mat& croppedImage)
{
    auto canvasWidthPixels = ROUND_INT(canvas.resolution()*canvas.width());
    auto canvasHeightPixels = ROUND_INT(canvas.resolution()*canvas.height());

    auto numPhotoRows = static_cast<size_t>(canvas.height() / (ps.photoHeightMM() + canvas.border()));
    auto numPhotoCols = static_cast<size_t>(canvas.width() / (ps.photoWidthMM() + canvas.border()));

    cv::Size tileSizePixels(ROUND_INT(canvas.resolution() * ps.photoWidthMM()), ROUND_INT(canvas.resolution() * ps.photoHeightMM()));

    // Resize input crop to the canvas output
    cv::Mat tileInCanvas;
    cv::resize(croppedImage, tileInCanvas, tileSizePixels);

    cv::Mat printPhoto(canvasHeightPixels, canvasWidthPixels, croppedImage.type());

    for (size_t row = 0; row < numPhotoRows; ++row)
    {
        for (size_t col = 0; col < numPhotoCols; ++col)
        {
            cv::Point topLeft(ROUND_INT(col*(ps.photoWidthMM() + canvas.border())*canvas.resolution()),
                ROUND_INT(row*(ps.photoHeightMM() + canvas.border())*canvas.resolution()));
            tileInCanvas.copyTo(printPhoto(cv::Rect(topLeft, tileSizePixels)));
        }
    }
    return printPhoto;
}

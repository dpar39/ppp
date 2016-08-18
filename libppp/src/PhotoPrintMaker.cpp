#include "PhotoStandard.h"
#include "PhotoPrintMaker.h"
#include "Geometry.h"

#include <opencv2/imgproc.hpp>

using namespace cv;

cv::Mat PhotoPrintMaker::cropPicture(const cv::Mat& originalImage,
                                     const cv::Point& crownPoint,
                                     const cv::Point& chinPoint,
                                     const PhotoStandard& ps)
{
    cv::Point2f centerCrop = (crownPoint + chinPoint) / 2.0f;
    auto faceHeightPix = cv::norm(crownPoint - chinPoint);

    auto cropHeightPix = ps.photoHeightMM() / ps.faceHeightMM() * faceHeightPix;
    auto cropWidthPix = ps.photoWidthMM() / ps.photoHeightMM() * cropHeightPix;

    auto pair1 = pointsAtDistanceParallelToCentreOf(crownPoint, chinPoint, cropHeightPix / 2.0);
    auto centerTop = cv::norm(cv::Point2d(crownPoint) - pair1.first) < cv::norm(cv::Point2d(crownPoint) - pair1.second) ? pair1.first : pair1.second;
    auto pair2 = pointsAtDistanceNormalToCentreOf(crownPoint, chinPoint, cropWidthPix / 2.0);
    auto centerLeft = cv::norm(cv::Point2d(crownPoint) - pair2.first) < cv::norm(cv::Point2d(crownPoint) - pair2.second) ? pair2.first : pair2.second;

    const Point2f srcs[3] = {centerCrop, centerLeft, centerTop};
    const Point2f dsts[3] = {Point2d(cropWidthPix / 2, cropHeightPix / 2), Point2d(0, cropHeightPix / 2), Point2d(cropWidthPix / 2, 0)};
    Mat tform = getAffineTransform(srcs, dsts);

    Mat cropImage;
    warpAffine(originalImage, cropImage, tform, cv::Size(ROUND_INT(cropWidthPix), ROUND_INT(cropHeightPix)));
    return cropImage;
}

#include "PassportStandard.h"
#include "Geometry.h"
#include <opencv2/imgproc/imgproc.hpp>
#include <memory>
using namespace cv;


PassportStandard::PassportStandard(double picWidth_mm, double picHeight_mm, double faceHeight_mm)
    : m_picHeight_mm(picHeight_mm), m_picWidth_mm(picWidth_mm), m_faceHeight_mm(faceHeight_mm)
{
}

cv::Mat PassportStandard::cropPicture(const cv::Mat& originalImage, const cv::Point& crownPoint, const cv::Point& chinPoint)
{
    Point2f centerCrop = (crownPoint + chinPoint) / 2.0f;
    auto faceHeightPix = cv::norm(crownPoint - chinPoint);

    auto cropHeightPix = m_picHeight_mm / m_faceHeight_mm * faceHeightPix;
    auto cropWidthPix = m_picWidth_mm / m_picHeight_mm * cropHeightPix;

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

std::shared_ptr<PassportStandard> PassportStandard::fromJson(rapidjson::Value& psConfig)
{
    auto picHeight = psConfig["pictureHeight"].GetDouble();
    auto picWidth = psConfig["pictureWidth"].GetDouble();
    auto faceLength = psConfig["faceHeight"].GetDouble();
    auto units = psConfig["units"].GetString();

    double unitsRatio_mm;
    if (units == "inch")
    {
        unitsRatio_mm = 25.4;
    }
    else if (units == "cm")
    {
        unitsRatio_mm = 10.0;
    }
    else if (units == "mm")
    {
        unitsRatio_mm = 1.0;
    }
    else
    {
        throw std::logic_error("Unknown input units");
    }

    return std::make_shared<PassportStandard>(picWidth * unitsRatio_mm, picHeight * unitsRatio_mm, faceLength * unitsRatio_mm);
}

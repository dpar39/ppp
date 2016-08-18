#include "LipsDetector.h"
#include "LandMarks.h"
#include "Geometry.h"

#include <algorithm>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/objdetect/objdetect.hpp>
#include "CommonHelpers.h"

using namespace cv;
using namespace std;

void LipsDetector::configure(rapidjson::Value& config)
{
    auto& lipsDetectorCfg = config["lipsDetector"];
    m_useHaarCascades = lipsDetectorCfg["useHaarCascade"].GetBool();
    if (!m_useHaarCascades)
    {
        return;
    }

    const string haarCascadeDir(config["haarCascadeDir"].GetString());
    const string haarCascadeFile(lipsDetectorCfg["haarCascade"].GetString());
    m_pMouthDetector = CommonHelpers::loadClassifier(haarCascadeDir, haarCascadeFile);
}

bool LipsDetector::detectLandMarks(const cv::Mat& origImage, ::LandMarks& landMarks)
{
    auto faceRectHeight = landMarks.vjFaceRect.height;
    auto leftEyePos = landMarks.eyeLeftPupil;
    auto rightEyePos = landMarks.eyeRightPupil;

    double distEyeLineToMouthCenter = ROUND_INT(0.40*faceRectHeight);
    auto mouthRoiWidth = ROUND_INT(0.40*faceRectHeight);
    auto mouthRoiHeight = ROUND_INT(0.30*faceRectHeight);

    auto eyeCentrePoint = (leftEyePos + rightEyePos) / 2;
    // Estimate center of the chin and crop a rectangle around it
    auto tmp = pointsAtDistanceNormalToCentreOf(leftEyePos, rightEyePos, distEyeLineToMouthCenter);

    auto mouthCenterPoint = tmp.first.y > tmp.second.y ? static_cast<Point>(tmp.first) : static_cast<Point>(tmp.second);
    Point mouthRoiLeftTop(mouthCenterPoint.x - mouthRoiWidth / 2, mouthCenterPoint.y - mouthRoiHeight / 2);
    Size mouthRoiSize(mouthRoiWidth, mouthRoiHeight);
    Rect mouthRoiRect(mouthRoiLeftTop, mouthRoiSize);
    auto mouthRoiImage = origImage(mouthRoiRect);

    Mat colorTformImage(mouthRoiHeight, mouthRoiWidth, CV_32F);

    auto dstBeg = colorTformImage.begin<float>();
    auto srcBeg = mouthRoiImage.begin<Vec3b>();
    auto srcEnd = mouthRoiImage.end<Vec3b>();

    std::transform(srcBeg, srcEnd, dstBeg, [](Vec3b pixel)
                   {
                       auto rgbSum = static_cast<float>(pixel[0]) + pixel[1] + pixel[2];
                       auto r = pixel[2] / rgbSum;
                       auto g = pixel[1] / rgbSum;
                       auto v = r / (r + g) * (1 - g / (r + g));
                       return v * v * 255;
                   });

    Mat u, v, binaryImg;
    colorTformImage.convertTo(u, CV_8UC1);

    threshold(u, v, 0, 255, THRESH_OTSU);

    morphologyEx(v, binaryImg, MORPH_CLOSE, getStructuringElement(MORPH_ELLIPSE, Size(7, 7)));

    std::vector<std::vector<Point>> contours;
    findContours(binaryImg, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, mouthRoiLeftTop);

    double maxArea1st = 0, maxArea2nd = 0;
    std::vector<std::vector<Point>>::iterator c1st, c2nd;

    // Select the two biggest regions (assuming they are the lips)
    for (auto c = contours.begin(); c != contours.end(); ++c)
    {
        double area = contourArea(*c);
        if (area > maxArea1st)
        {
            maxArea2nd = maxArea1st;
            c2nd = c1st;
            maxArea1st = area;
            c1st = c;
        }
    }

    auto candidates = contourLineIntersection(*c1st, eyeCentrePoint, mouthCenterPoint);

    Point leftCorner = Point(INT_MAX, 0), rightCorner = Point(INT_MIN, 0);
    for (auto& p : *c1st)
    {
        if (p.x < leftCorner.x)
        {
            leftCorner = p;
        }
        else if (p.x > rightCorner.x)
        {
            rightCorner = p;
        }
    }

    if (maxArea2nd > 0.5 * maxArea1st)
    {
        auto candidates2 = contourLineIntersection(*c2nd, eyeCentrePoint, mouthCenterPoint);
        candidates.insert(candidates.end(), candidates2.begin(), candidates2.end());
        for (auto& p : *c2nd)
        {
            if (p.x < leftCorner.x)
            {
                leftCorner = p;
            }
            else if (p.x > rightCorner.x)
            {
                rightCorner = p;
            }
        }
    }

    auto upperLip = *std::max_element(candidates.begin(), candidates.end(), [](Point2d& a, Point2d& b)
                                      {
                                          return a.y < b.y;
                                      });
    auto lowerLip = *std::max_element(candidates.begin(), candidates.end(), [](Point2d& a, Point2d& b)
                                      {
                                          return a.y > b.y;
                                      });

    landMarks.lipUpperCenter = upperLip;
    landMarks.lipLowerCenter = lowerLip;
    landMarks.lipLeftCorner = leftCorner;
    landMarks.lipRightCorner = rightCorner;
    return true;
}

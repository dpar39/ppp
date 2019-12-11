#include "LipsDetector.h"
#include "LandMarks.h"
#include "Utilities.h"

#include <algorithm>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/objdetect/objdetect.hpp>

using namespace cv;
using namespace std;

namespace ppp
{
void LipsDetector::configureInternal(rapidjson::Value & config)
{
    auto & lipsDetectorCfg = config["lipsDetector"];
    m_useHaarCascades = lipsDetectorCfg["useHaarCascade"].GetBool();

    m_useColorSegmentationAlgorithm = lipsDetectorCfg["useColorSegmentation"].GetBool();

    if (m_useHaarCascades)
    {
        const auto haarClassifierBase64 = lipsDetectorCfg["haarCascade"]["data"].GetString();
        m_pMouthCascadeClassifier = Utilities::loadClassifierFromBase64(haarClassifierBase64);
    }
    m_isConfigured = true;
}

bool LipsDetector::detectLandMarks(const Mat & inputImage, LandMarks & landmarks)
{
    auto faceRectHeight = landmarks.vjFaceRect.height;
    auto leftEyePos = landmarks.eyeLeftPupil;
    auto rightEyePos = landmarks.eyeRightPupil;

    double distEyeLineToMouthCenter = roundInteger(0.42 * faceRectHeight);
    auto mouthRoiWidth = roundInteger(0.45 * faceRectHeight);
    auto mouthRoiHeight = roundInteger(0.35 * faceRectHeight);

    auto eyeCentrePoint = (leftEyePos + rightEyePos) / 2.0;
    // Estimate center of the chin and crop a rectangle around it
    auto tmp = Utilities::pointsAtDistanceNormalToCentreOf(leftEyePos, rightEyePos, distEyeLineToMouthCenter);

    auto mouthCenterPoint = tmp.first.y > tmp.second.y ? static_cast<Point2d>(tmp.first)
                                                       : static_cast<Point2d>(tmp.second);
    Point mouthRoiLeftTop(roundInteger(mouthCenterPoint.x - mouthRoiWidth / 2),
                          roundInteger(mouthCenterPoint.y - mouthRoiHeight / 2));
    Size mouthRoiSize(mouthRoiWidth, mouthRoiHeight);
    Rect mouthRoiRect(mouthRoiLeftTop, mouthRoiSize);
    auto mouthRoiImage = inputImage(mouthRoiRect);

    if (m_useHaarCascades)
    {
        Mat mouthRoiImageGray;
        cvtColor(mouthRoiImage, mouthRoiImageGray, COLOR_BGR2GRAY);
        vector<Rect> mouthRects;
        vector<int> rejectLevels;
        vector<double> levelWeights;
        m_pMouthCascadeClassifier->detectMultiScale(mouthRoiImageGray,
                                                    mouthRects,
                                                    1.05,
                                                    3,
                                                    CASCADE_SCALE_IMAGE | CASCADE_FIND_BIGGEST_OBJECT,
                                                    mouthRoiSize / 4,
                                                    mouthRoiSize);

        if (!mouthRects.empty())
        {
            landmarks.vjMouthRect = mouthRects[0];
            landmarks.vjMouthRect.x += mouthRoiLeftTop.x;
            landmarks.vjMouthRect.y += mouthRoiLeftTop.y;
        }
    }

    if (m_useColorSegmentationAlgorithm)
    {
        Mat colorTformImage(mouthRoiHeight, mouthRoiWidth, CV_32F);

        auto dstBeg = colorTformImage.begin<float>();
        auto srcBeg = mouthRoiImage.begin<Vec3b>();
        auto srcEnd = mouthRoiImage.end<Vec3b>();

        std::transform(srcBeg, srcEnd, dstBeg, [](const Vec3b & pixel) {
            const auto rgbSum = static_cast<float>(pixel[0]) + pixel[1] + pixel[2];
            const auto r = pixel[2] / rgbSum;
            const auto g = pixel[1] / rgbSum;
            const auto v = r / (r + g) * (1 - g / (r + g));
            return v * v * 255;
        });

        Mat u, u2, v, binaryImg;
        colorTformImage.convertTo(u, CV_8UC1);
        blur(u, u2, Size(9, 3));

        threshold(u, v, 0, 255, THRESH_OTSU);
        morphologyEx(v, binaryImg, MORPH_CLOSE, getStructuringElement(MORPH_ELLIPSE, Size(7, 7)));

        std::vector<std::vector<Point>> contours;
        findContours(binaryImg, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE, mouthRoiLeftTop);

        double maxArea1st = 0, maxArea2nd = 0;
        std::vector<std::vector<Point>>::iterator c1st, c2nd;

        if (contours.empty())
        {
            // No contours were found
            return false;
        }

        // Select the two biggest regions (assuming they are the lips)
        for (auto c = contours.begin(); c != contours.end(); ++c)
        {
            // Ignore contour if it touches the border of the rectangle at the bottom
            if (std::any_of(c->begin(), c->end(), [mouthRoiWidth, mouthRoiHeight, &mouthRoiLeftTop](const Point & p) {
                    // return p.x == mouthRoiLeftTop.x || p.x >= mouthRoiLeftTop.x + mouthRoiWidth - 1
                    //        p.y == mouthRoiLeftTop.y || p.y >= mouthRoiLeftTop.y + mouthRoiHeight - 1;
                    return p.y >= mouthRoiLeftTop.y + mouthRoiHeight - 1;
                }))
            {
                continue;
            }
            // auto area = contourArea(*c);
            auto area = boundingRect(*c).width;

            if (area > maxArea1st)
            {
                maxArea2nd = maxArea1st;
                c2nd = c1st;
                maxArea1st = area;
                c1st = c;
            }
        }

        landmarks.lipContour1st = *c1st;
        if (maxArea2nd > 0)
        {
            landmarks.lipContour2nd = *c2nd;
        }

        auto candidates = Utilities::contourLineIntersection(*c1st, eyeCentrePoint, mouthCenterPoint);
        auto leftCorner = Point(INT_MAX, 0), rightCorner = Point(INT_MIN, 0);
        for (const auto & p : *c1st)
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
            auto candidates2 = Utilities::contourLineIntersection(*c2nd, eyeCentrePoint, mouthCenterPoint);
            candidates.insert(candidates.end(), candidates2.begin(), candidates2.end());
            for (auto & p : *c2nd)
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

        // auto upperLip = *std::max_element(candidates.begin(), candidates.end(), [](const Point2d &a, const Point2d &b)
        //                                  {
        //                                      return a.y < b.y;
        //                                  });
        // auto lowerLip = *std::max_element(candidates.begin(), candidates.end(), [](const Point2d &a, const Point2d &b)
        //                                  {
        //                                      return a.y > b.y;
        //                                  });

        // landMarks.lipUpperCenter = upperLip;
        // landMarks.lipLowerCenter = lowerLip;
        landmarks.lipLeftCorner = leftCorner;
        landmarks.lipRightCorner = rightCorner;
    }
    return true;
}

bool LipsDetector::getBeardMask(Mat & mouthAreaImage) const
{
    // mouthAreaImage
    return true;
}
} // namespace ppp

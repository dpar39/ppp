#include "LandMarks.h"
#include "EyeDetector.h"
#include "Geometry.h"
#include "CommonHelpers.h"

#include <queue>

#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/imgproc/imgproc.hpp>


using namespace std;
using namespace cv;

void EyeDetector::configure(rapidjson::Value& cfg)
{
    auto& edCfg = cfg["eyesDetector"];

    createCornerKernels();

    m_useHaarCascades = edCfg["useHaarCascade"].GetBool();

    if (!m_useHaarCascades)
    {
        return;
    }

    auto loadCascade = [&](const string& eyeName)
        {
            auto haarCascadeDir = cfg["haarCascadeDir"].GetString();
            auto haarCascadeFileName = edCfg[(string("haarCascade") + eyeName).c_str()].GetString();
            return CommonHelpers::loadClassifier(haarCascadeDir, haarCascadeFileName);
        };

    m_leftEyeCascadeClassifier = loadCascade("Left");
    m_rightEyeCascadeClassifier = loadCascade("Right");
}

bool EyeDetector::detectLandMarks(const cv::Mat& grayImage, LandMarks& landMarks)
{
    auto faceRect = landMarks.vjFaceRect;

    auto faceImage = grayImage(faceRect);

    if (kSmoothFaceImage)
    {
        double sigma = kSmoothFaceFactor * faceRect.width;
        GaussianBlur(faceImage, faceImage, cv::Size(0, 0), sigma);
    }
    //-- Find eye regions and draw them
    auto eyeRegionWidth = ROUND_INT(faceRect.width * m_widthRatio);
    auto eyeRegionHeight = ROUND_INT(faceRect.width * m_heightRatio);
    auto eyeRegionTop = ROUND_INT(faceRect.height * m_topFaceRatio);
    auto eyeRegionLeft = ROUND_INT(faceRect.width* m_sideFaceRatio);


    Rect leftEyeRegion(eyeRegionLeft, eyeRegionTop, eyeRegionWidth, eyeRegionHeight);
    Rect rightEyeRegion(faceRect.width - eyeRegionWidth - eyeRegionLeft,
                        eyeRegionTop, eyeRegionWidth, eyeRegionHeight);

    auto leftEyeImage = faceImage(leftEyeRegion);
    auto rightEyeImage = faceImage(rightEyeRegion);
    if (m_useHaarCascades)
    {
        auto leftEyeHaarRect =
            detectWithHaarCascadeClassifier(leftEyeImage, m_leftEyeCascadeClassifier.get());
        auto rightEyeHaarRect =
            detectWithHaarCascadeClassifier(rightEyeImage, m_rightEyeCascadeClassifier.get());

        landMarks.vjLeftEyeRect = leftEyeHaarRect;
        landMarks.vjRightEyeRect = rightEyeHaarRect;

        landMarks.vjLeftEyeRect.x += faceRect.x + leftEyeRegion.x;
        landMarks.vjLeftEyeRect.y += faceRect.y + leftEyeRegion.y;

        landMarks.vjRightEyeRect.x += faceRect.x + rightEyeRegion.x;
        landMarks.vjRightEyeRect.y += faceRect.y + rightEyeRegion.y;

        if (leftEyeHaarRect.width > 0 && leftEyeHaarRect.height > 0)
        {
            // Reduce the search area for the pupils
            leftEyeRegion.x += leftEyeHaarRect.x;
            leftEyeRegion.y += leftEyeHaarRect.y;
            leftEyeRegion.width = leftEyeHaarRect.width;
            leftEyeRegion.height = leftEyeHaarRect.height;
        }
        if (rightEyeHaarRect.width > 0 && rightEyeHaarRect.height > 0)
        {
            rightEyeRegion.x += rightEyeHaarRect.x;
            rightEyeRegion.y += rightEyeHaarRect.y;
            rightEyeRegion.width = rightEyeHaarRect.width;
            rightEyeRegion.height = rightEyeHaarRect.height;
        }
    }

    //-- Find Eye Centers
    auto leftPupil = findEyeCenter(faceImage(leftEyeRegion));
    auto rightPupil = findEyeCenter(faceImage(rightEyeRegion));

    // change eye centers to face coordinates
    rightPupil.x += rightEyeRegion.x + faceRect.x;
    rightPupil.y += rightEyeRegion.y + faceRect.y;
    leftPupil.x += leftEyeRegion.x + faceRect.x;
    leftPupil.y += leftEyeRegion.y + faceRect.y;

    //-- Find Eye Corners
    if (kEnableEyeCorner)
    {
        // Get corner regions
        cv::Rect leftRightCornerRegion(leftEyeRegion);
        leftRightCornerRegion.width -= leftPupil.x;
        leftRightCornerRegion.x += leftPupil.x;
        leftRightCornerRegion.height /= 2;
        leftRightCornerRegion.y += leftRightCornerRegion.height / 2;
        cv::Rect leftLeftCornerRegion(leftEyeRegion);
        leftLeftCornerRegion.width = leftPupil.x;
        leftLeftCornerRegion.height /= 2;
        leftLeftCornerRegion.y += leftLeftCornerRegion.height / 2;
        cv::Rect rightLeftCornerRegion(rightEyeRegion);
        rightLeftCornerRegion.width = rightPupil.x;
        rightLeftCornerRegion.height /= 2;
        rightLeftCornerRegion.y += rightLeftCornerRegion.height / 2;
        cv::Rect rightRightCornerRegion(rightEyeRegion);
        rightRightCornerRegion.width -= rightPupil.x;
        rightRightCornerRegion.x += rightPupil.x;
        rightRightCornerRegion.height /= 2;
        rightRightCornerRegion.y += rightRightCornerRegion.height / 2;

        cv::Point2f leftRightCorner = findEyeCorner(faceImage(leftRightCornerRegion), true, false);
        leftRightCorner.x += leftRightCornerRegion.x;
        leftRightCorner.y += leftRightCornerRegion.y;
        cv::Point2f leftLeftCorner = findEyeCorner(faceImage(leftLeftCornerRegion), true, true);
        leftLeftCorner.x += leftLeftCornerRegion.x;
        leftLeftCorner.y += leftLeftCornerRegion.y;
        cv::Point2f rightLeftCorner = findEyeCorner(faceImage(rightLeftCornerRegion), false, true);
        rightLeftCorner.x += rightLeftCornerRegion.x;
        rightLeftCorner.y += rightLeftCornerRegion.y;
        cv::Point2f rightRightCorner = findEyeCorner(faceImage(rightRightCornerRegion), false, false);
        rightRightCorner.x += rightRightCornerRegion.x;
        rightRightCorner.y += rightRightCornerRegion.y;
        circle(faceImage, leftRightCorner, 3, 200);
        circle(faceImage, leftLeftCorner, 3, 200);
        circle(faceImage, rightLeftCorner, 3, 200);
        circle(faceImage, rightRightCorner, 3, 200);
    }

    landMarks.eyeLeftPupil = leftPupil;
    landMarks.eyeRightPupil = rightPupil;

    return true;
}

cv::Rect EyeDetector::detectWithHaarCascadeClassifier(cv::Mat image, cv::CascadeClassifier* cc)
{
    vector<Rect> results;
    cc->detectMultiScale(image, results, 1.05, 3,
                         CV_HAAR_SCALE_IMAGE | CV_HAAR_FIND_BIGGEST_OBJECT);
    if (results.empty() || results.size() > 1)
        return Rect();
    return results.front();
}

void EyeDetector::createCornerKernels()
{
    float kEyeCornerKernel[4][6] = {
        {-1, -1, -1, 1, 1, 1},
        {-1, -1, -1, -1, 1, 1},
        {-1, -1, -1, -1, 0, 3},
        {1, 1, 1, 1, 1, 1},
    };

    rightCornerKernel = make_shared<cv::Mat>(4, 6,CV_32F, kEyeCornerKernel);
    leftCornerKernel = make_shared<cv::Mat>(4, 6, CV_32F);

    // flip horizontally
    cv::flip(*rightCornerKernel, *leftCornerKernel, 1);
}

cv::Point EyeDetector::findEyeCenter(const cv::Mat& image)
{
    cv::Mat eyeROIUnscaled = image;
    cv::Mat eyeROI;
    scaleToFastSize(eyeROIUnscaled, eyeROI);

    //-- Find the gradient
    cv::Mat gradientX = computeMatXGradient(eyeROI);
    cv::Mat gradientY = computeMatXGradient(eyeROI.t()).t();

    //-- Normalize and threshold the gradient, compute all the magnitudes
    cv::Mat mags = matrixMagnitude(gradientX, gradientY);
    //compute the threshold
    double gradientThresh = computeDynamicThreshold(mags, kGradientThreshold);
    //double gradientThresh = kGradientThreshold;
    //double gradientThresh = 0;
    //normalize
    for (int y = 0; y < eyeROI.rows; ++y)
    {
        double* Xr = gradientX.ptr<double>(y);
        double* Yr = gradientY.ptr<double>(y);
        const double* Mr = mags.ptr<double>(y);
        for (int x = 0; x < eyeROI.cols; ++x)
        {
            double gX = Xr[x], gY = Yr[x];
            double magnitude = Mr[x];
            if (magnitude > gradientThresh)
            {
                Xr[x] = gX / magnitude;
                Yr[x] = gY / magnitude;
            }
            else
            {
                Xr[x] = 0.0;
                Yr[x] = 0.0;
            }
        }
    }

    //-- Create a blurred and inverted image for weighting
    cv::Mat weight;
    GaussianBlur(eyeROI, weight, cv::Size(kWeightBlurSize, kWeightBlurSize), 0, 0);

    weight = -weight + 255;

    //-- Run the algorithm!
    cv::Mat outSum = cv::Mat::zeros(eyeROI.rows, eyeROI.cols, CV_64F);
    // for each possible gradient location
    // Note: these loops are reversed from the way the paper does them
    // it evaluates every possible center for each gradient location instead of
    // every possible gradient location for every center.

    for (int y = 0; y < weight.rows; ++y)
    {
        const unsigned char* Wr = weight.ptr<unsigned char>(y);
        const double *Xr = gradientX.ptr<double>(y), *Yr = gradientY.ptr<double>(y);
        for (int x = 0; x < weight.cols; ++x)
        {
            double gX = Xr[x], gY = Yr[x];
            if (gX == 0.0 && gY == 0.0)
                continue;
            testPossibleCentersFormula(x, y, Wr[x], gX, gY, outSum);
        }
    }
    // scale all the values down, basically averaging them
    double numGradients = (weight.rows * weight.cols);
    cv::Mat out;
    outSum.convertTo(out, CV_32F, 1.0 / numGradients);

    //-- Find the maximum point
    cv::Point maxP;
    double maxVal;
    cv::minMaxLoc(out, nullptr, &maxVal, nullptr, &maxP);

    //-- Flood fill the edges
    if (kEnablePostProcess)
    {
        cv::Mat floodClone;
        //double floodThresh = computeDynamicThreshold(out, 1.5);
        double floodThresh = maxVal * kPostProcessThreshold;
        cv::threshold(out, floodClone, floodThresh, 0.0f, cv::THRESH_TOZERO);

        cv::Mat mask = floodKillEdges(floodClone);
        //imshow(debugWindow + " Mask",mask);
        //imshow(debugWindow,out);
        // redo max
        cv::minMaxLoc(out, nullptr, &maxVal, nullptr, &maxP, mask);
    }
    return unscalePoint(maxP, Rect(Point(0, 0), image.size()));
}

cv::Mat EyeDetector::eyeCornerMap(const cv::Mat& region, bool left, bool left2)
{
    cv::Mat cornerMap;

    cv::Size sizeRegion = region.size();
    cv::Range colRange(sizeRegion.width / 4, sizeRegion.width * 3 / 4);
    cv::Range rowRange(sizeRegion.height / 4, sizeRegion.height * 3 / 4);

    cv::Mat miRegion(region, rowRange, colRange);

    cv::filter2D(miRegion, cornerMap, CV_32F,
                 (left && !left2) || (!left && !left2) ? *leftCornerKernel : *rightCornerKernel);
    return cornerMap;
}

cv::Point2f EyeDetector::findEyeCorner(cv::Mat region, bool left, bool left2)
{
    cv::Mat cornerMap = eyeCornerMap(region, left, left2);

    cv::Point maxP;
    cv::minMaxLoc(cornerMap, nullptr, nullptr, nullptr, &maxP);

    cv::Point2f maxP2;
    maxP2 = findSubpixelEyeCorner(cornerMap, maxP);

    return maxP2;
}

cv::Point2f EyeDetector::findSubpixelEyeCorner(cv::Mat region, cv::Point maxP)
{
    cv::Size sizeRegion = region.size();

    cv::Mat cornerMap(sizeRegion.height * 10, sizeRegion.width * 10, CV_32F);

    cv::resize(region, cornerMap, cornerMap.size(), 0, 0, cv::INTER_CUBIC);

    cv::Point maxP2;
    cv::minMaxLoc(cornerMap, nullptr, nullptr, nullptr, &maxP2);

    return cv::Point2f(static_cast<float>(sizeRegion.width / 2 + maxP2.x / 10),
                       static_cast<float>(sizeRegion.height / 2 + maxP2.y / 10));
}

cv::Mat floodKillEdges(cv::Mat& mat);

cv::Point EyeDetector::unscalePoint(cv::Point p, cv::Rect origSize)
{
    float ratio = static_cast<float>(kFastEyeWidth) / origSize.width;
    int x = ROUND_INT(p.x / ratio);
    int y = ROUND_INT(p.y / ratio);
    return cv::Point(x, y);
}

void EyeDetector::scaleToFastSize(const cv::Mat& src, cv::Mat& dst)
{
    cv::resize(src, dst, cv::Size(kFastEyeWidth, static_cast<int>(static_cast<float>(kFastEyeWidth) / src.cols * src.rows)));
}

cv::Mat EyeDetector::computeMatXGradient(const cv::Mat& mat)
{
    cv::Mat out(mat.rows, mat.cols, CV_64F);

    for (int y = 0; y < mat.rows; ++y)
    {
        const uchar* Mr = mat.ptr<uchar>(y);
        double* Or = out.ptr<double>(y);

        Or[0] = Mr[1] - Mr[0];
        for (int x = 1; x < mat.cols - 1; ++x)
        {
            Or[x] = (Mr[x + 1] - Mr[x - 1]) / 2.0;
        }
        Or[mat.cols - 1] = Mr[mat.cols - 1] - Mr[mat.cols - 2];
    }
    return out;
}

void EyeDetector::testPossibleCentersFormula(int x, int y, unsigned char weight, double gx, double gy, cv::Mat& out)
{
    // for all possible centers
    for (int cy = 0; cy < out.rows; ++cy)
    {
        double* Or = out.ptr<double>(cy);
        for (int cx = 0; cx < out.cols; ++cx)
        {
            if (x == cx && y == cy)
            {
                continue;
            }
            // create a vector from the possible center to the gradient origin
            double dx = x - cx;
            double dy = y - cy;
            // normalize d
            double magnitude = sqrt((dx * dx) + (dy * dy));
            dx = dx / magnitude;
            dy = dy / magnitude;
            double dotProduct = dx * gx + dy * gy;
            dotProduct = std::max(0.0, dotProduct);
            // square and multiply by the weight
            if (kEnableWeight)
            {
                Or[cx] += dotProduct * dotProduct * (weight / kWeightDivisor);
            }
            else
            {
                Or[cx] += dotProduct * dotProduct;
            }
        }
    }
}

bool floodShouldPushPoint(const cv::Point& np, const cv::Mat& mat)
{
    return np.x >= 0 && np.x < mat.cols && np.y >= 0 && np.y < mat.rows;
}

cv::Mat EyeDetector::floodKillEdges(cv::Mat& mat)
{
    rectangle(mat, cv::Rect(0, 0, mat.cols, mat.rows), 255);

    cv::Mat mask(mat.rows, mat.cols, CV_8U, 255);
    std::queue<cv::Point> toDo;
    toDo.push(cv::Point(0, 0));
    while (!toDo.empty())
    {
        cv::Point p = toDo.front();
        toDo.pop();
        if (mat.at<float>(p) == 0.0f)
            continue;

        // add in every direction
        cv::Point np(p.x + 1, p.y); // right
        if (floodShouldPushPoint(np, mat)) toDo.push(np);
        np.x = p.x - 1;
        np.y = p.y; // left
        if (floodShouldPushPoint(np, mat)) toDo.push(np);
        np.x = p.x;
        np.y = p.y + 1; // down
        if (floodShouldPushPoint(np, mat)) toDo.push(np);
        np.x = p.x;
        np.y = p.y - 1; // up
        if (floodShouldPushPoint(np, mat)) toDo.push(np);
        // kill it
        mat.at<float>(p) = 0.0f;
        mask.at<uchar>(p) = 0;
    }
    return mask;
}

cv::Mat EyeDetector::matrixMagnitude(const cv::Mat& matX, const cv::Mat& matY)
{
    cv::Mat mags(matX.rows, matX.cols, CV_64F);
    for (int y = 0; y < matX.rows; ++y)
    {
        const double *Xr = matX.ptr<double>(y), *Yr = matY.ptr<double>(y);
        double* Mr = mags.ptr<double>(y);
        for (int x = 0; x < matX.cols; ++x)
        {
            double gX = Xr[x], gY = Yr[x];
            double magnitude = sqrt((gX * gX) + (gY * gY));
            Mr[x] = magnitude;
        }
    }
    return mags;
}

double EyeDetector::computeDynamicThreshold(const cv::Mat& mat, double stdDevFactor)
{
    cv::Scalar stdMagnGrad, meanMagnGrad;
    cv::meanStdDev(mat, meanMagnGrad, stdMagnGrad);
    double stdDev = stdMagnGrad[0] / sqrt(mat.rows * mat.cols);
    return stdDevFactor * stdDev + meanMagnGrad[0];
}

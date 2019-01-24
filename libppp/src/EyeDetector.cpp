#include "LandMarks.h"
#include "EyeDetector.h"
#include <queue>
#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "Utilities.h"

using namespace std;

void EyeDetector::configure(rapidjson::Value& cfg)
{
    auto& edCfg = cfg["eyesDetector"];

    createCornerKernels();

    m_useHaarCascades = edCfg["useHaarCascade"].GetBool();

    if (m_useHaarCascades)
    {
        auto loadCascade = [&edCfg](const string& eyeName)
        {
            auto & haarCascade = edCfg[(string("haarCascade") + eyeName).c_str()];
            auto xmlBase64Data(haarCascade["data"].GetString());
            return Utilities::loadClassifierFromBase64(xmlBase64Data);
        };
        m_leftEyeCascadeClassifier = loadCascade("Left");
        m_rightEyeCascadeClassifier = loadCascade("Right");
    }
}

bool EyeDetector::detectLandMarks(const cv::Mat& grayImage, LandMarks& landMarks)
{
    const auto& faceRect = landMarks.vjFaceRect;

    if (faceRect.width <= 10 && faceRect.height <= 10)
    {
        throw std::runtime_error("Face rectangle is too small or not defined");
    }

    auto faceImage = grayImage(faceRect);

    if (m_smoothFaceImage)
    {
        double sigma = m_smoothFaceFactor * faceRect.width;
        GaussianBlur(faceImage, faceImage, cv::Size(0, 0), sigma);
    }
    //-- Find eye regions and draw them
    auto eyeRegionWidth = ROUND_INT(faceRect.width * m_widthRatio);
    auto eyeRegionHeight = ROUND_INT(faceRect.width * m_heightRatio);
    auto eyeRegionTop = ROUND_INT(faceRect.height * m_topFaceRatio);
    auto eyeRegionLeft = ROUND_INT(faceRect.width* m_sideFaceRatio);


    cv::Rect leftEyeRegion(eyeRegionLeft, eyeRegionTop, eyeRegionWidth, eyeRegionHeight);
    cv::Rect rightEyeRegion(faceRect.width - eyeRegionWidth - eyeRegionLeft,
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
    auto leftEyeCenter = findEyeCenter(faceImage(leftEyeRegion));
    auto rightEyeCenter = findEyeCenter(faceImage(rightEyeRegion));

    //-- If eye center touches or is very close to the eye ROI apply fallback method
    validateAndApplyFallbackIfRequired(leftEyeRegion.size(), leftEyeCenter);
    validateAndApplyFallbackIfRequired(rightEyeRegion.size(), rightEyeCenter);

    // Change eye centers to face coordinates
    rightEyeCenter.x += rightEyeRegion.x + faceRect.x;
    rightEyeCenter.y += rightEyeRegion.y + faceRect.y;
    leftEyeCenter.x += leftEyeRegion.x + faceRect.x;
    leftEyeCenter.y += leftEyeRegion.y + faceRect.y;

    landMarks.eyeLeftPupil = leftEyeCenter;
    landMarks.eyeRightPupil = rightEyeCenter;

    return true;
}

void EyeDetector::validateAndApplyFallbackIfRequired(const cv::Size &eyeRoiSize, cv::Point &eyeCenter)
{
    if (eyeRoiSize.width <= eyeCenter.x || eyeRoiSize.height < eyeCenter.y)
    {
        throw std::logic_error("Detected eye position is outside the specifiied eye ROI");
    }

    const auto epsilon = std::min(eyeRoiSize.width, eyeRoiSize.height) * 0.1;
    if (eyeRoiSize.width- eyeCenter.x < epsilon || eyeCenter.x < epsilon
        || eyeRoiSize.height - eyeCenter.y < epsilon || eyeCenter.y < epsilon)
    {
        eyeCenter.x = ROUND_INT(eyeRoiSize.width / 2.0);
        eyeCenter.y = ROUND_INT(eyeRoiSize.height / 2.0);
    }
}

cv::Rect EyeDetector::detectWithHaarCascadeClassifier(const cv::Mat &image, cv::CascadeClassifier* cc)
{
    vector<cv::Rect> results;
    cc->detectMultiScale(image, results, 1.05, 3,
                         cv::CASCADE_SCALE_IMAGE | cv::CASCADE_FIND_BIGGEST_OBJECT);
    if (results.empty() || results.size() > 1)
    {
        return cv::Rect();
    }
    return results.front();
}

void EyeDetector::createCornerKernels()
{
    m_rightCornerKernel = (cv::Mat_<float>(4, 6) <<
        -1, -1, -1,  1, 1, 1,
        -1, -1, -1, -1, 1, 1,
        -1, -1, -1, -1, 0, 3,
         1,  1,  1,  1, 1, 1);
    m_xGradKernel = (cv::Mat_<float>(3, 3) << 0, 0, 0, -0.5, 0, 0.5, 0, 0, 0);

    m_yGradKernel = m_xGradKernel.t();

    // flip horizontally
    cv::flip(m_rightCornerKernel, m_leftCornerKernel, 1);
}

cv::Point EyeDetector::findEyeCenter(const cv::Mat& eyeROIUnscaled) const
{
    cv::Mat eyeRoi;
    scaleToFastSize(eyeROIUnscaled, eyeRoi);

    //-- Find the gradient
    cv::Mat gradientX, gradientY;

    cv::filter2D(eyeRoi, gradientX, CV_64F, m_xGradKernel);
    cv::filter2D(eyeRoi, gradientY, CV_64F, m_yGradKernel);

    //-- Normalize and threshold the gradient, compute all the magnitudes
    auto mags = matrixMagnitude(gradientX, gradientY);
    //compute the threshold
    auto gradientThresh = computeDynamicThreshold(mags, kGradientThreshold);
    //double gradientThresh = kGradientThreshold;
    //double gradientThresh = 0;
    //normalize
    for (auto y = 0; y < eyeRoi.rows; ++y)
    {
        auto Xr = gradientX.ptr<double>(y);
        auto Yr = gradientY.ptr<double>(y);
        const double* Mr = mags.ptr<double>(y);

        for (auto x = 0; x < eyeRoi.cols; ++x)
        {
            auto gX = Xr[x], gY = Yr[x];
            auto magnitude = Mr[x];
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
    GaussianBlur(eyeRoi, weight, cv::Size(kWeightBlurSize, kWeightBlurSize), 0, 0);

    weight = -weight + 255;

    //-- Run the algorithm!
    cv::Mat outSum = cv::Mat::zeros(eyeRoi.rows, eyeRoi.cols, CV_64F);
    // for each possible gradient location
    // Note: these loops are reversed from the way the paper does them
    // it evaluates every possible center for each gradient location instead of
    // every possible gradient location for every center.

    for (int y = 0; y < weight.rows; ++y)
    {
        const unsigned char* Wr = weight.ptr<unsigned char>(y);
        const double *Xr = gradientX.ptr<double>(y),
        *Yr = gradientY.ptr<double>(y);
        for (int x = 0; x < weight.cols; ++x)
        {
            double gX = Xr[x], gY = Yr[x];
            if (gX == 0.0 && gY == 0.0)
            {
                continue;
            }
            testPossibleCentersFormula(x, y, Wr[x], gX, gY, outSum);
        }
    }
    // scale all the values down, basically averaging them
    double numGradients = weight.rows * weight.cols;
    cv::Mat out;
    outSum.convertTo(out, CV_32F, 1.0 / numGradients);

    //-- Find the maximum point
    cv::Point maxP;
    double maxVal;
    cv::minMaxLoc(out, nullptr, &maxVal, nullptr, &maxP);

    //-- Flood fill the edges
    if (m_enablePostProcess)
    {
        cv::Mat floodClone;
        //double floodThresh = computeDynamicThreshold(out, 1.5);
        auto floodThresh = maxVal * m_postProcessThreshold;
        cv::threshold(out, floodClone, floodThresh, 0.0f, cv::THRESH_TOZERO);

        auto mask = floodKillEdges(floodClone);
        // redo max
        cv::minMaxLoc(out, nullptr, &maxVal, nullptr, &maxP, mask);
    }
    return unscalePoint(maxP, cv::Rect(cv::Point(0, 0), eyeROIUnscaled.size()));
}

cv::Point EyeDetector::unscalePoint(cv::Point p, cv::Rect origSize) const
{
    auto ratio = static_cast<float>(kFastEyeWidth) / origSize.width;
    auto x = ROUND_INT(p.x / ratio);
    auto y = ROUND_INT(p.y / ratio);
    return cv::Point(x, y);
}

void EyeDetector::scaleToFastSize(const cv::Mat& src, cv::Mat& dst) const
{
    cv::resize(src, dst, cv::Size(kFastEyeWidth,
        static_cast<int>(static_cast<float>(kFastEyeWidth) / src.cols * src.rows)));
}

void EyeDetector::testPossibleCentersFormula(int x, int y, unsigned char weight, double gx, double gy, cv::Mat& out) const
{
    // for all possible centers
    for (auto cy = 0; cy < out.rows; ++cy)
    {
        auto Or = out.ptr<double>(cy);
        for (auto cx = 0; cx < out.cols; ++cx)
        {
            if (x == cx && y == cy)
            {
                continue;
            }
            // create a vector from the possible center to the gradient origin
            double dx = x - cx;
            double dy = y - cy;
            // normalize d
            auto magnitude = sqrt((dx * dx) + (dy * dy));
            dx = dx / magnitude;
            dy = dy / magnitude;
            auto dotProduct = dx * gx + dy * gy;
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

cv::Mat EyeDetector::floodKillEdges(cv::Mat& mat) const
{
    rectangle(mat, cv::Rect(0, 0, mat.cols, mat.rows), 255);

    cv::Mat mask(mat.rows, mat.cols, CV_8U, 255);
    std::queue<cv::Point> toDo;
    toDo.push(cv::Point(0, 0));
    while (!toDo.empty())
    {
        auto p = toDo.front();
        toDo.pop();
        if (mat.at<float>(p) == 0.0f)
        {
            continue;
        }

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

cv::Mat EyeDetector::matrixMagnitude(const cv::Mat& matX, const cv::Mat& matY) const
{
    cv::Mat magnitude(matX.rows, matX.cols, CV_64F);
    cv::sqrt(matX.mul(matX) + matY.mul(matY), magnitude);
    return magnitude;
}

double EyeDetector::computeDynamicThreshold(const cv::Mat& mat, double stdDevFactor) const
{
    cv::Scalar stdMagnGrad, meanMagnGrad;
    cv::meanStdDev(mat, meanMagnGrad, stdMagnGrad);
    auto stdDev = stdMagnGrad[0] / sqrt(mat.rows * mat.cols);
    return stdDevFactor * stdDev + meanMagnGrad[0];
}

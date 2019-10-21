#include "FacePoseEstimator.h"
#include "LandMarks.h"
#include <opencv2/calib3d.hpp>

FacePoseEstimator::FacePoseEstimator()
{
    m_modelPoints.clear();
    m_modelPoints.emplace_back(0.0f, 0.0f, 0.0f); // Nose tip
    m_modelPoints.emplace_back(0.0f, -330.0f, -65.0f); // Chin
    m_modelPoints.emplace_back(-225.0f, 170.0f, -135.0f); // Left eye left corner
    m_modelPoints.emplace_back(225.0f, 170.0f, -135.0f); // Right eye right corner
    m_modelPoints.emplace_back(-150.0f, -150.0f, -125.0f); // Left Mouth corner
    m_modelPoints.emplace_back(150.0f, -150.0f, -125.0f); // Right mouth corner
}

cv::Vec3d FacePoseEstimator::estimatePose(const LandMarks & landMarks,
                                          const double focalLength,
                                          const cv::Point2d focalCenter)
{
    using cv::Point2d;
    using cv::Point3d;

    // 2D image points. If you change the image, you need to change vector
    const std::vector<Point2d> imagePoints {
        landMarks.noseTip, // Nose tip
        landMarks.chinPoint, // Chin
        landMarks.eyeLeftCorner, // Left eye left corner
        landMarks.eyeRightCorner, // Right eye right corner
        landMarks.lipLeftCorner, // Left Mouth corner
        landMarks.lipRightCorner // Right mouth corner
    };

    m_cameraMatrix = (cv::Mat_<double>(3, 3) << focalLength, 0, focalCenter.x, 0, focalLength, focalCenter.y, 0, 0, 1);

    // auto cm1 = (cv::Mat(3, 3) << focalLength, 0.0, focalCenter.x, 0.0, focalLength, focalCenter.y, 0.0, 0.0, 1.0);
    // cameraMatrix = cm1;

    const cv::Mat distCoeffs = cv::Mat::zeros(4, 1, cv::DataType<double>::type); // Assuming no lens distortion

    // Output rotation and translation

    // Solve for pose
    const auto success
        = solvePnP(m_modelPoints, imagePoints, m_cameraMatrix, distCoeffs, m_rotationVector, m_translationVector);

    cv::Mat rotationMatrix;
    cv::Rodrigues(m_rotationVector, rotationMatrix);

    auto r = rotationMatrix;

    float sy = sqrt(r.at<double>(0, 0) * r.at<double>(0, 0) + r.at<double>(1, 0) * r.at<double>(1, 0));

    const auto singular = sy < 1e-6; // If

    double x, y, z;
    if (!singular)
    {
        x = atan2(r.at<double>(2, 1), r.at<double>(2, 2));
        y = atan2(-r.at<double>(2, 0), sy);
        z = atan2(r.at<double>(1, 0), r.at<double>(0, 0));
    }
    else
    {
        x = atan2(-r.at<double>(1, 2), r.at<double>(1, 1));
        y = atan2(-r.at<double>(2, 0), sy);
        z = 0;
    }
    const auto deg = [](const double rad) { return rad * 180.0 / 3.141592653589793238463; };
    return cv::Vec3d(deg(x), deg(y), deg(z));
}

void FacePoseEstimator::projectPoint(const std::vector<cv::Point3d> & point3ds,
                                     std::vector<cv::Point2d> & point2ds) const
{
    static const cv::Mat distCoeffs = cv::Mat::zeros(4, 1, cv::DataType<double>::type); // Assuming no lens distortion
    cv::projectPoints(point3ds, m_rotationVector, m_translationVector, m_cameraMatrix, distCoeffs, point2ds);
}

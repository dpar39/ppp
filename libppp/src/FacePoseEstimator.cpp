#include "FacePoseEstimator.h"
#include "LandMarks.h"
#include <iostream>
#include <opencv2/calib3d.hpp>
#include <opencv2/highgui.hpp>

void FacePoseEstimator::configure()
{
    m_modelPoints.clear();
    m_modelPoints.emplace_back(0.0f, 0.0f, 0.0f); // Nose tip
    m_modelPoints.emplace_back(0.0f, -330.0f, -65.0f); // Chin
    m_modelPoints.emplace_back(-225.0f, 170.0f, -135.0f); // Left eye left corner
    m_modelPoints.emplace_back(225.0f, 170.0f, -135.0f); // Right eye right corner
    m_modelPoints.emplace_back(-150.0f, -150.0f, -125.0f); // Left Mouth corner
    m_modelPoints.emplace_back(150.0f, -150.0f, -125.0f); // Right mouth corner
}

void FacePoseEstimator::estimatePose(const LandMarks & landMarks, double focalLength, cv::Point2d focalCenter) const
{
    using cv::Point2d;
    using cv::Point3d;

    // 2D image points. If you change the image, you need to change vector
    std::vector<cv::Point2d> image_points;

    // landMarks.allLandmarks[];

    image_points.emplace_back(359, 391); // Nose tip
    image_points.emplace_back(399, 561); // Chin
    image_points.emplace_back(337, 297); // Left eye left corner
    image_points.emplace_back(513, 301); // Right eye right corner
    image_points.emplace_back(345, 465); // Left Mouth corner
    image_points.emplace_back(453, 469); // Right mouth corner

    // 3D model points.

    // Camera internals
    // double focal_length = im.cols; // Approximate focal length.
    // Point2d center = cv::Point2d(im.cols / 2, im.rows / 2);
    const cv::Mat cameraMatrix
        = (cv::Mat_<double>(3, 3) << focalLength, 0, focalCenter.x, 0, focalLength, focalCenter.y, 0, 0, 1);

    const cv::Mat distCoeffs = cv::Mat::zeros(4, 1, cv::DataType<double>::type); // Assuming no lens distortion

    // Output rotation and translation
    cv::Mat rotationVector; // Rotation in axis-angle form
    cv::Mat translationVector;

    // Solve for pose
    cv::solvePnP(m_modelPoints, image_points, cameraMatrix, distCoeffs, rotationVector, translationVector);
}

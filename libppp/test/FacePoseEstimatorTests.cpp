#include <gtest/gtest.h>
#include <opencv2/calib3d.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace cv;

class FacePoseEstimatorTests : public testing::Test
{

protected:
};

TEST_F(FacePoseEstimatorTests, canConfigure)
{

    cv::Mat im;

    // Project a 3D point (0, 0, 1000.0) onto the image plane.
    // We use this to draw a line sticking out of the nose

    const std::vector<Point3d> noseEndPoint3D(1, Point3d(0, 0, 1000.0));
    std::vector<Point2d> noseEndPoint2D;

    // cv::projectPoints(noseEndPoint3D, rotationVector, translationVector, cameraMatrix, distCoeffs, noseEndPoint2D);

    // for (int i = 0; i < image_points.size(); i++)
    //{
    //    cv::circle(im, image_points[i], 3, Scalar(0, 0, 255), -1);
    //}

    // cv::line(im, image_points[0], noseEndPoint2D[0], cv::Scalar(255, 0, 0), 2);

    //// Display image.
    // cv::imshow("Output", im);
    // cv::waitKey(0);
}

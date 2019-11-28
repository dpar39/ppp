#include "EasyExif.h"
#include "FacePoseEstimator.h"
#include "ImageStore.h"
#include "PppEngine.h"
#include "TestHelpers.h"
#include <gtest/gtest.h>
#include <opencv2/calib3d.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace cv;

namespace ppp
{
class FacePoseEstimatorTests : public testing::Test
{
protected:
};

TEST_F(FacePoseEstimatorTests, canConfigure)
{
    const auto imageFilePath = resolvePath("research/mugshot_frontal_original_all/078_frontal.jpg");
    //  const auto imageFilePath = resolvePath("research/my_database/000.jpg");

    ImageStore store;
    const auto imgKey = store.setImage(imageFilePath);
    auto img = store.getImage(imgKey);

    const auto exif = store.getExifInfo(imgKey);
    const auto focalLengthPix = exif->focalLengthPix();

    const auto landMarks = loadLandmarks(imageFilePath);

    FacePoseEstimator fpe;

    const auto center = Point2d(img.cols / 2, img.rows / 2);

    const auto rot = fpe.estimatePose(landMarks, focalLengthPix, center);

    // Project a 3D point (0, 0, 1000.0) onto the image plane.
    // We use this to draw a line sticking out of the nose
    const std::vector<Point3d> noseEndPoint3D(1, Point3d(0, 0, 1000.0));
    std::vector<Point2d> noseEndPoint2D;

    fpe.projectPoint(noseEndPoint3D, noseEndPoint2D);

    renderLandmarksOnImage(img, landMarks);

    cv::line(img, landMarks->noseTip, noseEndPoint2D[0], cv::Scalar(255, 0, 0), 2);
}
} // namespace ppp

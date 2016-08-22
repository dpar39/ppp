#include <gtest/gtest.h>
#include <vector>
#include <functional>

#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <PppEngine.h>
#include <LandMarks.h>
#include <Geometry.h>

#include "TestHelpers.h"

#include <FaceDetector.h>
#include <EyeDetector.h>
#include <LipsDetector.h>
#include <rapidjson/document.h>

#include "common.h"

using namespace cv;

// image name prefix, rgb image, gray image, annotated landmarks, detected landmarks
typedef std::function<bool(const std::string&, cv::Mat&, cv::Mat&, LandMarks&, LandMarks&)> DetectionCallback;

#define LANDMARK_POINT(mat, row) cv::Point(ROUND_INT((mat).at<float>((row), 0)), ROUND_INT((mat).at<float>((row), 1)))
#define IN_ROI(r, p) (((p).x > (r).x) && ((p).x < ((r).x + (r).width)) && ((p).y > (r).y) && ((p).y < ((r).y + (r).height)))

const std::string g_defaultConfigPath(resolvePath("share/config.json"));

void processDatabase(DetectionCallback callback, bool annotateResults = false)
{
    using namespace std;
    auto imageDir = resolvePath("research/sample_test_images");
    imageDir = resolvePath("research/mugshot_frontal_original_all");
    vector<string> imageFileNames;
    getImageFiles(imageDir, imageFileNames);

    vector<int> imageIndices;

    int count = 0;
    imageIndices.resize(imageFileNames.size());
    generate_n(imageIndices.begin(), imageFileNames.size(), [&]()
               {
                   return ++count;
               });

    //imageIndices = { 76, 88, 92, 115 };
    //imageIndices = {38, 40, 56, 69, 76, 88, 98, 114, 122, 125, 129 };

    //imageIndices = { 61 };
    vector<int> excludeList = {
        56, // Glasses with some reflexion
        74, // Eyes totally closed
        76, // Glasses with some reflexion
        88, // Glasses, eyes mostly closed
        92, // Right eye fail <<<<<<<<<
        115, // Old guy, eyes very closed
        121
    };

    for (auto idx : imageIndices)
    {
        if (find(excludeList.begin(), excludeList.end(), idx) != excludeList.end())
        {
            continue;
        }

        auto imageFileName = imageFileNames[idx - 1];
        auto inputImage = cv::imread(imageFileName);
        cv::Mat grayImage;
        cv::cvtColor(inputImage, grayImage, CV_BGR2GRAY);

        auto imagePrefix = imageFileName.substr(0, imageFileName.find_last_of('.'));

        string annotatedLandMarkFiles = imagePrefix + ".pos";
        cv::Mat landMarksAnn;
        ASSERT_TRUE(importTextMatrix(annotatedLandMarkFiles, landMarksAnn));

        LandMarks annotations, results;
        annotations.eyeLeftPupil = LANDMARK_POINT(landMarksAnn, 6);
        annotations.eyeRightPupil = LANDMARK_POINT(landMarksAnn, 9);
        annotations.lipLeftCorner = LANDMARK_POINT(landMarksAnn, 14);
        annotations.lipRightCorner = LANDMARK_POINT(landMarksAnn, 15);
        annotations.crownPoint = LANDMARK_POINT(landMarksAnn, 0);
        annotations.chinPoint = LANDMARK_POINT(landMarksAnn, 16);

        bool success = callback(imagePrefix, inputImage, grayImage, annotations, results);
        EXPECT_TRUE(success);

        if (annotateResults)
        {
            Scalar annnotationColor(0, 30, 255);
            Scalar detectionColor(250, 30, 0);

            circle(inputImage, annotations.eyeLeftPupil, 5, annnotationColor, 2);
            circle(inputImage, annotations.eyeRightPupil, 5, annnotationColor, 2);
            circle(inputImage, annotations.eyeLeftPupil, 5, annnotationColor, 2);
            circle(inputImage, annotations.eyeRightPupil, 5, annnotationColor, 2);
            circle(inputImage, annotations.crownPoint, 5, annnotationColor, 2);
            circle(inputImage, annotations.chinPoint, 5, annnotationColor, 2);

            rectangle(inputImage, results.vjFaceRect, Scalar(0, 128, 0), 2);
            rectangle(inputImage, results.vjLeftEyeRect, Scalar(0xA0, 0x52, 0x2D), 3);
            rectangle(inputImage, results.vjRightEyeRect, Scalar(0xA0, 0x52, 0x2D), 3);

            circle(inputImage, results.eyeLeftPupil, 5, detectionColor, 2);
            circle(inputImage, results.eyeRightPupil, 5, detectionColor, 2);

            circle(inputImage, results.lipLeftCorner, 5, detectionColor, 2);
            circle(inputImage, results.lipRightCorner, 5, detectionColor, 2);

            circle(inputImage, results.crownPoint, 5, detectionColor, 2);
            circle(inputImage, results.chinPoint, 5, detectionColor, 2);
        }
    }
}

rapidjson::Document readConfigFromFile(const std::string& configPath)
{
    std::ifstream fs(configPath, std::ios_base::in);
    std::string jsonstr((std::istreambuf_iterator<char>(fs)),
                        std::istreambuf_iterator<char>());
    rapidjson::Document d;
    d.Parse(jsonstr.c_str());
    return d;
}

TEST(PppEngine, CanConfigure)
{
    PppEngine pa;
    rapidjson::Document config;
    EXPECT_NO_THROW(config = readConfigFromFile(g_defaultConfigPath));
    EXPECT_NO_THROW(pa.configure(config));
}

TEST(PppEngine, DISABLED_CanDetectFaces)
{
    auto config = readConfigFromFile(g_defaultConfigPath);
    FaceDetector fd;
    fd.configure(config);

    auto process = [&](const std::string& imagePrefix, cv::Mat& rgbImage, cv::Mat& grayImage,
            LandMarks& annotations, LandMarks& detectedLandMarks) -> bool
        {
            EXPECT_TRUE(fd.detectLandMarks(grayImage, detectedLandMarks))
                    << "Error detecting face in " << imagePrefix;

            // Check that the rectangle contains both eyes and mouth points
            auto faceRect = detectedLandMarks.vjFaceRect;

            EXPECT_TRUE(IN_ROI(faceRect, annotations.eyeLeftPupil));
            EXPECT_TRUE(IN_ROI(faceRect, annotations.eyeRightPupil));
            EXPECT_TRUE(IN_ROI(faceRect, annotations.lipLeftCorner));
            EXPECT_TRUE(IN_ROI(faceRect, annotations.lipRightCorner));

            return true;
        };

    processDatabase(process);
}

TEST(PppEngine, DISABLED_CanDetectEyesAndLips)
{
    using namespace std;
    auto config = readConfigFromFile(g_defaultConfigPath);

    FaceDetector fd;
    EyeDetector ed;
    LipsDetector ld;

    fd.configure(config);
    ed.configure(config);
    ld.configure(config);

    vector<double> leftEyeErrors, rightEyeErrors;
    vector<double> leftLipsErrors, rightLipsErrors;

    auto process = [&](const std::string& imagePrefix, cv::Mat& rgbImage, cv::Mat& grayImage,
            LandMarks& annotations, LandMarks& detectedLandMarks) -> bool
        {
            EXPECT_TRUE(fd.detectLandMarks(grayImage, detectedLandMarks))
                    << "Error detecting face in " << imagePrefix;

            EXPECT_TRUE(ed.detectLandMarks(grayImage, detectedLandMarks))
                    << "Error detecting eyes in " << imagePrefix;

            EXPECT_TRUE(ld.detectLandMarks(rgbImage, detectedLandMarks))
                    << "Error detecting lips in " << imagePrefix;

            auto success = IN_ROI(detectedLandMarks.vjLeftEyeRect, annotations.eyeLeftPupil) &&
                IN_ROI(detectedLandMarks.vjRightEyeRect, annotations.eyeRightPupil);

            EXPECT_TRUE(success) << "Failed " << imagePrefix << endl;

            double maxEyeAllowedError = 16;
            auto leftEyeError = cv::norm(detectedLandMarks.eyeLeftPupil - annotations.eyeLeftPupil);
            auto rightEyeError = cv::norm(detectedLandMarks.eyeRightPupil - annotations.eyeRightPupil);

            double maxLipCornerAllowedError = 25;
            auto leftLipsError = cv::norm(detectedLandMarks.lipLeftCorner - annotations.lipLeftCorner);
            auto rightLipsError = cv::norm(detectedLandMarks.lipRightCorner - annotations.lipRightCorner);

            leftEyeErrors.push_back(leftEyeError);
            rightEyeErrors.push_back(rightEyeError);
            leftLipsErrors.push_back(leftLipsError);
            rightLipsErrors.push_back(rightLipsError);

            if (leftEyeError > maxEyeAllowedError)
            {
                cout << " Problem detecting left eye in " << imagePrefix << endl;
            }
            if (rightEyeError > maxEyeAllowedError)
            {
                cout << " Problem detecting right eye in " << imagePrefix << endl;
            }

            if (leftLipsError > maxLipCornerAllowedError)
            {
                cout << " Problem detecting left lips corner in " << imagePrefix << endl;
            }
            if (rightLipsError > maxLipCornerAllowedError)
            {
                cout << " Problem detecting right lips corner in " << imagePrefix << endl;
            }
            return true;
        };

    processDatabase(process);
}

TEST(PppEngine, DISABLED_TestEndToEndLandmarkDetection)
{
    using namespace std;
    auto config = readConfigFromFile(g_defaultConfigPath);
    PppEngine engine;
    engine.configure(config);

    vector<double> relativeErrors;

    auto process = [&](const std::string& imagePrefix, cv::Mat& rgbImage, cv::Mat& grayImage,
            LandMarks& annotations, LandMarks& detectedLandMarks) -> bool
        {
            auto imageKey = engine.setInputImage(rgbImage);
            engine.detectLandMarks(imageKey, detectedLandMarks);

            // Validate chin-crown distance error
            auto expectedDistance = cv::norm(annotations.chinPoint - annotations.crownPoint);
            auto actualDistance = cv::norm(detectedLandMarks.chinPoint - detectedLandMarks.crownPoint);

            const double maxError = (36.0 - 32.0) / 34.0;
            auto relError = cv::abs(expectedDistance - actualDistance) / expectedDistance;
            relativeErrors.push_back(relError);
            auto accepted = relError < maxError;

            if (!accepted)
            {
                cout << " Estimation of face height too large for image " << imagePrefix << endl;
            }
            return accepted;
        };

    processDatabase(process);
}

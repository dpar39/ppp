#pragma once

#include <fstream>
#include <functional>

#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <rapidjson/document.h>

#include "LandMarks.h"
#include "Geometry.h"

#ifdef POCO_STATIC
#include <Poco/DirectoryIterator.h>

inline void getImageFiles(const std::string &testImagesDir, std::vector<std::string> &imageFilenames)
{
    std::vector<std::string> supportedImageExtensions = { "jpg", /*"png",*/ "bmp" };

    using Poco::DirectoryIterator;
    using Poco::Path;

    DirectoryIterator itr(testImagesDir);
    DirectoryIterator end;
    while (itr != end)
    {
        if (!itr->isFile())
        {
            continue;
        }
        auto fileExt = Poco::Path(itr->path()).getExtension();
        std::transform(fileExt.begin(), fileExt.end(), fileExt.begin(), ::tolower);
        if (std::find(supportedImageExtensions.begin(), supportedImageExtensions.end(), fileExt) != supportedImageExtensions.end())
        {
            imageFilenames.push_back(itr->path());
        }
        ++itr;
    }
}

inline std::string resolvePath(const std::string &relPath)
{
    auto baseDir = Poco::Path(Poco::Path::current());
    while(baseDir.depth() > 1 )
    {
        auto currPath = baseDir;
        auto combinePath = Poco::File(currPath.append(relPath));
        if (combinePath.exists())
        {
            return combinePath.path();
        }
        baseDir = baseDir.parent();
    }
    return std::string();
}

inline std::string pathCombine(const std::string &prefix, const std::string &suffix)
{
    return Poco::Path(prefix, suffix).toString();
}

#else
#include <filesystem>

inline std::string resolvePath(const std::string& relPath)
{
    namespace fs = std::tr2::sys;
    auto baseDir = fs::current_path();
    while (baseDir.has_parent_path())
    {
        auto combinePath = baseDir / relPath;
        if (fs::exists(combinePath))
        {
            return combinePath.string();
        }
        baseDir = baseDir.parent_path();
    }
    return std::string();
}

inline std::string pathCombine(const std::string& prefix, const std::string& suffix)
{
    namespace fs = std::tr2::sys;
    return (fs::path(prefix) / fs::path(suffix)).string();
}

inline void getImageFiles(const std::string& testImagesDir, std::vector<std::string>& imageFilenames)
{
    std::vector<std::string> supportedImageExtensions = {".jpg", /*".png",*/ ".bmp"};

    namespace fs = std::tr2::sys;
    fs::directory_iterator endIter;
    if (fs::exists(fs::path(testImagesDir)) && fs::is_directory(fs::path(testImagesDir)))
    {
        for (fs::directory_iterator itr(testImagesDir); itr != endIter; ++itr)
        {
            if (!fs::is_regular_file(itr->status()))
            {
                continue;
            }
            auto filePath = itr->path();
            auto fileExt = filePath.extension().string();
            std::transform(fileExt.begin(), fileExt.end(), fileExt.begin(), ::tolower);
            if (std::find(supportedImageExtensions.begin(), supportedImageExtensions.end(), fileExt) != supportedImageExtensions.end())
            {
                imageFilenames.push_back(filePath.string());
            }
        }
    }
}
#endif

inline std::string getFileName(const std::string &filePath)
{
    char separator[2] = { '/', '\\' };
    auto it = std::find_first_of(filePath.rbegin(), filePath.rend(), separator, separator + 2);
    if (it != filePath.rend())
    {
        std::string fileName(filePath.rbegin(), it);
        std::reverse(fileName.begin(), fileName.end());
        return fileName;
    }
    return filePath;
}

inline bool importTextMatrix(const std::string& txtFileName, cv::Mat& output)
{
    std::ifstream textFile(txtFileName);
    std::string str;
    int numRows = 0;
    int numCols = 0;
    std::vector<float> data;

    while (getline(textFile, str))
    {
        std::stringstream stream(str);
        int numValuesInRow = 0;
        while (1)
        {
            float value;
            stream >> value;
            if (!stream)
                break;
            data.push_back(value);
            numValuesInRow++;
        }
        if (numRows == 0)
        {
            numCols = numValuesInRow;
        }
        else if (numValuesInRow == 0)
        {
            break; // Empty line
        }
        else if (numCols != numValuesInRow)
        {
            textFile.close();
            return false;
        }
        numRows++;
    }
    auto dataPtr = data.data();
    output = cv::Mat(numRows, numCols, CV_32F, dataPtr, cv::Mat::AUTO_STEP).clone();
    textFile.close();
    return true;
}

inline void verifyEqualImages(const cv::Mat& expected, const cv::Mat& actual)
{
    ASSERT_EQ(expected.size, actual.size) << "Images have different sizes";
    ASSERT_EQ(0, cv::countNonZero(cv::abs(expected - actual))) << "Images are not the same pixel by pixel";
}

inline rapidjson::Document readConfigFromFile(const std::string& configFile = "")
{
    auto configFilePath = configFile.empty()
                              ? resolvePath("libppp/share/config.json")
                              : configFile;

    std::ifstream fs(configFilePath, std::ios_base::in);
    std::string jsonstr((std::istreambuf_iterator<char>(fs)),
                        std::istreambuf_iterator<char>());
    rapidjson::Document d;
    d.Parse(jsonstr.c_str());
    return d;
}

// image name prefix, rgb image, gray image, annotated landmarks, detected landmarks
typedef std::function<bool(const std::string&, cv::Mat&, cv::Mat&, LandMarks&, LandMarks&)> DetectionCallback;

#define LANDMARK_POINT(mat, row) cv::Point(ROUND_INT((mat).at<float>((row), 0)), ROUND_INT((mat).at<float>((row), 1)))
#define IN_ROI(r, p) (((p).x > (r).x) && ((p).x < ((r).x + (r).width)) && ((p).y > (r).y) && ((p).y < ((r).y + (r).height)))


inline void processDatabase(DetectionCallback callback, std::vector<std::string> ignoredImages, bool annotateResults = false)
{
    using namespace std;
    auto imageDir = resolvePath("research/sample_test_images");
    imageDir = resolvePath("research/mugshot_frontal_original_all");
    vector<string> imageFileNames;
    getImageFiles(imageDir, imageFileNames);

    for (const auto& imageFileName : imageFileNames)
    {
        if (find_if(ignoredImages.begin(), ignoredImages.end(), [&imageFileName](const std::string &ignoreImageFile)
        {
            return imageFileName.find(ignoreImageFile) != std::string::npos;
        }) != ignoredImages.end())
        {
            continue; // Skip processing this image
        };

        auto inputImage = cv::imread(imageFileName);
        cv::Mat grayImage;
        cv::cvtColor(inputImage, grayImage, CV_BGR2GRAY);

        auto imagePrefix = imageFileName.substr(0, imageFileName.find_last_of('.'));
        auto annotatedLandMarkFiles = imagePrefix + ".pos";
        cv::Mat landMarksAnn;
        ASSERT_TRUE(importTextMatrix(annotatedLandMarkFiles, landMarksAnn));

        LandMarks annotations, results;
        annotations.eyeLeftPupil = LANDMARK_POINT(landMarksAnn, 6);
        annotations.eyeRightPupil = LANDMARK_POINT(landMarksAnn, 9);
        annotations.lipLeftCorner = LANDMARK_POINT(landMarksAnn, 14);
        annotations.lipRightCorner = LANDMARK_POINT(landMarksAnn, 15);
        annotations.crownPoint = LANDMARK_POINT(landMarksAnn, 0);
        annotations.chinPoint = LANDMARK_POINT(landMarksAnn, 16);

        auto imageName = getFileName(imageFileName);

        auto success = callback(imageName, inputImage, grayImage, annotations, results);
        if (!success)
        {
            success = false;
        }
        EXPECT_TRUE(success);

        if (annotateResults)
        {
            cv::Scalar annnotationColor(0, 30, 255);
            cv::Scalar detectionColor(250, 30, 0);

            circle(inputImage, annotations.eyeLeftPupil, 5, annnotationColor, 2);
            circle(inputImage, annotations.eyeRightPupil, 5, annnotationColor, 2);
            circle(inputImage, annotations.eyeLeftPupil, 5, annnotationColor, 2);
            circle(inputImage, annotations.eyeRightPupil, 5, annnotationColor, 2);
            circle(inputImage, annotations.crownPoint, 5, annnotationColor, 2);
            circle(inputImage, annotations.chinPoint, 5, annnotationColor, 2);

            rectangle(inputImage, results.vjFaceRect, cv::Scalar(0, 128, 0), 2);
            rectangle(inputImage, results.vjLeftEyeRect, cv::Scalar(0xA0, 0x52, 0x2D), 3);
            rectangle(inputImage, results.vjRightEyeRect, cv::Scalar(0xA0, 0x52, 0x2D), 3);
            //rectangle(inputImage, results.vjMouth, Scalar(0xA0, 0x52, 0x2D), 3);

            circle(inputImage, results.eyeLeftPupil, 5, detectionColor, 2);
            circle(inputImage, results.eyeRightPupil, 5, detectionColor, 2);

            circle(inputImage, results.lipLeftCorner, 5, detectionColor, 2);
            circle(inputImage, results.lipRightCorner, 5, detectionColor, 2);

            circle(inputImage, results.crownPoint, 5, detectionColor, 2);
            circle(inputImage, results.chinPoint, 5, detectionColor, 2);
        }
    }
}

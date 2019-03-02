#pragma once

#include "LandMarks.h"
#include <functional>
#include <rapidjson/document.h>

// image name prefix, rgb image, gray image, annotated landmarks, detected landmarks
typedef std::function<bool(const std::string &, cv::Mat &, cv::Mat &, const LandMarks &, LandMarks &)> DetectionCallback;

struct ResultData
{
    ResultData(const std::string & imgName, const LandMarks & annotated, const LandMarks & detected, bool isSuccess)
    : imageName(imgName)
    , annotation(annotated)
    , detection(detected)
    , isSuccess(isSuccess)
    {
    }

    std::string imageName;
    LandMarks annotation;
    LandMarks detection;
    bool isSuccess;
};

#define IN_ROI(r, p)                                                                                                   \
    (((p).x > (r).x) && ((p).x < ((r).x + (r).width)) && ((p).y > (r).y) && ((p).y < ((r).y + (r).height)))

std::string resolvePath(const std::string & relPath);

std::string pathCombine(const std::string & prefix, const std::string & suffix);

void getImageFiles(const std::string & testImagesDir, std::vector<std::string> & imageFilenames);

std::string getFileName(const std::string & filePath);

bool importSCFaceLandMarks(const std::string & txtFileName, cv::Mat & output);

void verifyEqualImages(const cv::Mat & expected, const cv::Mat & actual);

void readConfigFromFile(const std::string & configFile, std::string & configString);

void processDatabase(const DetectionCallback & callback,
                     const std::vector<std::string> & ignoredImages,
                     const std::string & landmarksPath,
                     std::vector<ResultData> & rd);

void adjustCrownChinCoeffs(const std::vector<LandMarks> & groundTruthAnnotations);

template <typename T>
static double median(std::vector<T> scores)
{
    double median;
    size_t size = scores.size();
    sort(scores.begin(), scores.end());
    if (size % 2 == 0)
    {
        median = (scores[size / 2 - 1] + scores[size / 2]) / 2;
    }
    else
    {
        median = scores[size / 2];
    }
    return median;
}

#pragma once

#include "CommonHelpers.h"
#include "LandMarks.h"

#include <opencv2/core/core.hpp>

#include <functional>
#include <utility>

namespace ppp
{
FWD_DECL(IImageStore)

// image name prefix, rgb image, gray image, annotated landmarks, detected landmarks
using DetectionCallback = std::function<std::pair<bool, cv::Mat>(const std::string &, const LandMarks &, LandMarks &)>;

struct ResultData final
{
    ResultData(std::string imgName, LandMarks annotated, LandMarks detected, bool isSuccess)
    : imageName(std::move(imgName))
    , annotation(std::move(annotated))
    , detection(std::move(detected))
    , isSuccess(isSuccess)
    {
    }

    std::string imageName;
    LandMarks annotation;
    LandMarks detection;
    bool isSuccess;
};

std::string resolvePath(const std::string & relPath);

std::string pathCombine(const std::string & prefix, const std::string & suffix);

void getImageFiles(const std::string & testImagesDir, std::vector<std::string> & imageFileNames);

std::string getFileName(const std::string & filePath);

bool importSCFaceLandMarks(const std::string & txtFileName, cv::Mat & output);

void benchmarkValidate(const cv::Mat & actualImage, const std::string & suffix = "");

void verifyEqualImages(const cv::Mat & expected, const cv::Mat & actual);

void readConfigFromFile(const std::string & configFile, std::string & configString);

void processDatabase(const DetectionCallback & callback,
                     const std::vector<std::string> & ignoredImages,
                     const std::string & landmarksPath,
                     std::vector<ResultData> & rd);

void adjustCrownChinCoefficients(const std::vector<LandMarks> & groundTruthAnnotations);

void persistLandmarks(const std::string & imageFilePath, const LandMarks & detectedLandmarks);

void loadLandmarks(const std::string & imageFilePath, LandMarks & detectedLandmarks);

void renderLandmarksOnImage(cv::Mat & image, const LandMarks & lm);

template <typename T>
static double median(std::vector<T> scores)
{
    double median;
    const size_t size = scores.size();
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
} // namespace ppp

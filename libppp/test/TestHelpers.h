#pragma once

#include "CommonHelpers.h"
#include "LandMarks.h"

#include <opencv2/core/core.hpp>

#include <functional>
#include <utility>

namespace ppp
{
FWD_DECL(IImageStore)
FWD_DECL(ConfigLoader)

using DetectionCallback
    = std::function<std::tuple<bool, cv::Mat, LandMarksSPtr>(const std::string &, const LandMarksSPtr &)>;

struct ResultData final
{
    ResultData(std::string imgName, LandMarksSPtr annotated, LandMarksSPtr detected, bool isSuccess)
    : imageName(std::move(imgName))
    , annotation(std::move(annotated))
    , detection(std::move(detected))
    , isSuccess(isSuccess)
    {
    }

    std::string imageName;
    LandMarksSPtr annotation;
    LandMarksSPtr detection;
    bool isSuccess;
};

std::string resolvePath(const std::string & relPath);

std::string pathCombine(const std::string & prefix, const std::string & suffix);

void getImageFiles(const std::string & testImagesDir, std::vector<std::string> & imageFileNames);

std::string getFileName(const std::string & filePath);

void benchmarkValidate(const cv::Mat & actualImage, const std::string & suffix = "");

void verifyEqualImages(const cv::Mat & expected, const cv::Mat & actual);

ConfigLoaderSPtr getConfigLoader(const std::string & configFile = "");

void processDatabase(const DetectionCallback & callback,
                     const std::vector<std::string> & ignoredImages,
                     const std::string & landmarksPath,
                     std::vector<ResultData> & rd);

void adjustCrownChinCoefficients(const std::vector<LandMarks> & groundTruthAnnotations);

LandMarksSPtr loadLandmarks(const std::string & imageFilePath);

void renderLandmarksOnImage(cv::Mat & image, const LandMarksSPtr & lm);

bool loadJson(const std::string & filePath, rapidjson::Document & d);

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

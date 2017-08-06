#pragma once

#include <functional>
#include <rapidjson/document.h>
#include "LandMarks.h"

// image name prefix, rgb image, gray image, annotated landmarks, detected landmarks
typedef std::function<bool(const std::string&, cv::Mat&, cv::Mat&, const LandMarks&, LandMarks&)> DetectionCallback;

struct ResultsData
{
    std::vector<LandMarks> annotatedLandMarks;
    std::vector<LandMarks> detectedLandMarks;
};

#define IN_ROI(r, p) (((p).x > (r).x) && ((p).x < ((r).x + (r).width)) && ((p).y > (r).y) && ((p).y < ((r).y + (r).height)))

std::string resolvePath(const std::string& relPath);

std::string pathCombine(const std::string& prefix, const std::string& suffix);

void getImageFiles(const std::string& testImagesDir, std::vector<std::string>& imageFilenames);

std::string getFileName(const std::string& filePath);

bool importSCFaceLandMarks(const std::string& txtFileName, cv::Mat& output);

void verifyEqualImages(const cv::Mat& expected, const cv::Mat& actual);

rapidjson::Document readConfigFromFile(const std::string& configFile = "");

void processDatabase(DetectionCallback callback, const std::vector<std::string> &ignoredImages, const std::string &landmarksPath, ResultsData &dr);


void adjustCrownChinCoeffs(const std::vector<LandMarks>& groundTruthAnnotations);

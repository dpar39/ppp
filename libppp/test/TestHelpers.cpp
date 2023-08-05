#include <fstream>
#include <regex>

#include <gtest/gtest.h>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "ConfigLoader.h"
#include "ImageStore.h"
#include "PppEngine.h"
#include "TestHelpers.h"
#include "Utilities.h"

#include <map>

#include <filesystem>
namespace fs = std::filesystem;

#define LANDMARK_POINT(mat, row)                                                                                       \
    cv::Point(roundInteger((mat).at<float>((row), 0)), roundInteger((mat).at<float>((row), 1)))

namespace ppp {
std::string resolvePath(const std::string & relPath)
{
    for (auto baseDir : { fs::current_path(), fs::path("/src") }) {
        while (baseDir.has_parent_path()) {
            auto combinePath = baseDir / relPath;
            if (exists(combinePath)) {
                return combinePath.string();
            }
            if (baseDir == baseDir.parent_path()) {
                break;
            }
            baseDir = baseDir.parent_path();
        }
    }
    return {};
}

std::string pathCombine(const std::string & prefix, const std::string & suffix)
{
    return (fs::path(prefix) / fs::path(suffix)).string();
}

void getImageFiles(const std::string & testImagesDir, std::vector<std::string> & imageFileNames)
{
    std::vector<std::string> supportedImageExtensions = { ".jpg", /*".png",*/ ".bmp" };
    fs::directory_iterator endIter;
    if (exists(fs::path(testImagesDir)) && is_directory(fs::path(testImagesDir))) {
        for (fs::directory_iterator itr(testImagesDir); itr != endIter; ++itr) {
            if (!is_regular_file(itr->status())) {
                continue;
            }
            auto filePath = itr->path();
            auto fileExt = filePath.extension().string();
            std::transform(fileExt.begin(), fileExt.end(), fileExt.begin(), tolower);
            if (std::find(supportedImageExtensions.begin(), supportedImageExtensions.end(), fileExt)
                != supportedImageExtensions.end()) {
                imageFileNames.push_back(filePath.string());
            }
        }
    }
}

std::string getFileName(const std::string & filePath)
{
    return fs::path(filePath).filename().string();
}

std::string getDirectory(const std::string & fullPath)
{
    return fs::path(fullPath).parent_path().string();
}

/**
 * \brief Loads the landmarks manually annotated from a CSV file
 * \param csvFilePath Path to the CSV file containing the annotation in VIA format
 * \return image name to landMarks map
 */
std::map<std::string, LandMarksSPtr> importLandMarks(const std::string & csvFilePath)
{
    const std::string pattern
        = "(.*\\.(jpg|JPG|png|PNG)),\\d+,\"\\{\\}\",6,(\\d),\".*\"\"cx\"\":(\\d+),\"\"cy\"\":(\\d+)\\}\",\"\\{\\}\"";
    std::regex e(pattern);

    std::ifstream t(csvFilePath);
    std::string csv_content((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());
    const auto imageDir = getDirectory(csvFilePath);

    std::map<std::string, LandMarksSPtr> result;
    for (auto it = std::sregex_iterator(csv_content.begin(), csv_content.end(), e); it != std::sregex_iterator(); ++it) {
        const auto & m = *it;
        for (const auto & x : m) {
            std::string imageName = m[1];
            auto fullImagePath = (fs::path(imageDir) / fs::path(imageName)).string();
            auto landmarkIdx = stoi(m[3]);
            cv::Point point(stoi(m[4]), stoi(m[5]));

            if (result.find(fullImagePath) == result.end()) {
                result[fullImagePath] = LandMarks::create();
            }
            auto & lm = *result[fullImagePath];

            switch (landmarkIdx) {
                case 0:
                    lm.crownPoint = point;
                    break;
                case 1:
                    lm.chinPoint = point;
                    break;
                case 2:
                    lm.eyeLeftPupil = point;
                    break;
                case 3:
                    lm.eyeRightPupil = point;
                    break;
                case 4:
                    lm.lipLeftCorner = point;
                    break;
                case 5:
                    lm.lipRightCorner = point;
                    break;
                default:
                    throw std::runtime_error("Invalid landmark index when reading from CSV file");
            }
        }
    }
    return result;
}

void benchmarkValidate(const cv::Mat & actualImage, const std::string & suffix)
{
    const std::string testName = ::testing::UnitTest::GetInstance()->current_test_info()->name();
    const auto filename = testName + suffix + ".png";
    const auto expectedImageFilePath = pathCombine(resolvePath("libppp/test/data"), filename);
    if (fs::exists(expectedImageFilePath)) {
        const auto expectedImage = cv::imread(expectedImageFilePath);
        const auto numDistinctPixels = countNonZero(sum(cv::abs(expectedImage - actualImage)));
        EXPECT_EQ(numDistinctPixels, 0) << "Actual image differs to image in file " << expectedImageFilePath;
    } else {
        cv::imwrite(expectedImageFilePath, actualImage);
        FAIL() << "Benchmark file did not exist!";
    }
}

void verifyEqualImages(const cv::Mat & expected, const cv::Mat & actual)
{
    ASSERT_EQ(expected.size, actual.size) << "Images have different sizes";
    const auto diff = expected != actual;
    cv::Mat graymat;
    cv::cvtColor(diff, graymat, cv::COLOR_BGR2GRAY, 1);
    ASSERT_EQ(0, cv::countNonZero(graymat)) << "Images are not the same pixel by pixel";
}

ConfigLoaderSPtr getConfigLoader(const std::string & configFile)
{
    auto configFilePath = configFile;
    if (configFile.empty()) {
        configFilePath = resolvePath("libppp/share/config.json");
    }
    return std::make_shared<ConfigLoader>(configFilePath);
}

void readConfigFromFile(const std::string & configFile, std::string & configString)
{
    const auto configFilePath = configFile.empty() ? resolvePath("libppp/share/config.json") : configFile;
    std::ifstream fs(configFilePath, std::ios_base::in);
    configString.assign(std::istreambuf_iterator<char>(fs), std::istreambuf_iterator<char>());
}

bool pathsContain(const std::vector<std::string> & filePaths, const std::string & fileName)
{
    return find_if(filePaths.begin(),
                   filePaths.end(),
                   [&fileName](const std::string & f) { return fileName.find(f) != std::string::npos; })
        != filePaths.end();
}

std::vector<ResultData> processDatabase(const DetectionCallback & callback,
                                        const std::string & landmarksPath,
                                        const std::vector<std::string> & includedImages,
                                        const std::vector<std::string> & excludedImages)
{
#ifdef _DEBUG
    constexpr auto annotateResults = true;
#else
    constexpr auto ANNOTATE_RESULTS = false;
#endif

    std::vector<ResultData> rd;

    const auto annotationFile = resolvePath(landmarksPath);
    const auto landMarksSet = importLandMarks(annotationFile);

    const auto imageStore = std::make_shared<ImageStore>();
    for (const auto & annotatedImage : landMarksSet) {
        const auto & imageFileName = annotatedImage.first;
        const auto & annotations = annotatedImage.second;

        if (imageFileName.find("049_frontal") == std::string::npos) {
            // continue;
        }

        if (pathsContain(excludedImages, imageFileName)) {
            continue; // Skip processing this image
        }

        if (!includedImages.empty() && !pathsContain(includedImages, imageFileName)) {
            continue;
        }

        auto imagePrefix = imageFileName.substr(0, imageFileName.find_last_of('.'));
        auto annotatedLandMarkFiles = imagePrefix + ".pos";
        cv::Mat landMarksAnn;

        auto imageName = getFileName(imageFileName);

        auto [isSuccess, inputImage, landmarks] = callback(imageFileName, annotations);

        if (ANNOTATE_RESULTS) {
            cv::Scalar annotationColor(0, 30, 255);
            cv::Scalar detectionColor(250, 30, 0);

            circle(inputImage, annotations->eyeLeftPupil, 5, annotationColor, 2);
            circle(inputImage, annotations->eyeRightPupil, 5, annotationColor, 2);
            circle(inputImage, annotations->lipLeftCorner, 5, annotationColor, 2);
            circle(inputImage, annotations->lipRightCorner, 5, annotationColor, 2);
            circle(inputImage, annotations->crownPoint, 5, annotationColor, 2);
            circle(inputImage, annotations->chinPoint, 5, annotationColor, 2);

            rectangle(inputImage, landmarks->vjFaceRect, cv::Scalar(0, 128, 0), 2);
            rectangle(inputImage, landmarks->vjLeftEyeRect, cv::Scalar(0xA0, 0x52, 0x2D), 3);
            rectangle(inputImage, landmarks->vjRightEyeRect, cv::Scalar(0xA0, 0x52, 0x2D), 3);

            polylines(inputImage,
                      std::vector<std::vector<cv::Point>> { landmarks->lipContour1st, landmarks->lipContour2nd },
                      true,
                      detectionColor);
            rectangle(inputImage, landmarks->vjMouthRect, cv::Scalar(0xA0, 0x52, 0x2D), 3);

            circle(inputImage, landmarks->eyeLeftPupil, 5, detectionColor, 2);
            circle(inputImage, landmarks->eyeRightPupil, 5, detectionColor, 2);

            circle(inputImage, landmarks->lipLeftCorner, 5, detectionColor, 2);
            circle(inputImage, landmarks->lipRightCorner, 5, detectionColor, 2);

            circle(inputImage, landmarks->crownPoint, 5, detectionColor, 2);
            circle(inputImage, landmarks->chinPoint, 5, detectionColor, 2);
        }

        rd.emplace_back(imageFileName, annotations, landmarks, isSuccess);
    }
    return rd;
}

std::string getLandMarkFileFor(const std::string & imageFilePath)
{
    const auto lastPathSep = imageFilePath.find_last_of("/\\");
    const auto imageFileName = lastPathSep != std::string::npos ? imageFilePath.substr(lastPathSep + 1) : imageFilePath;
    const auto testDataDir = resolvePath("libppp/test/data");
    return testDataDir + "/" + imageFileName + ".json";
}

LandMarksSPtr loadLandmarks(const std::string & imageFilePath)
{
    using namespace std;
    const auto landmarksFilePath = getLandMarkFileFor(imageFilePath);
    rapidjson::Document d;

    if (loadJson(landmarksFilePath, d)) {
        auto landmarks = make_shared<LandMarks>();
        landmarks->fromJson(d);
        return landmarks;
    }

    // Compute new landmarks
    static auto s_configured = false;
    static PppEngine s_engine;
    if (!s_configured) {
        std::string configString;
        readConfigFromFile("", configString);
        s_configured = s_engine.configure(configString);
    }
    const auto & store = s_engine.getImageStore();
    const auto imgKey = store->setImage(imageFilePath);
    EXPECT_TRUE(s_engine.detectLandMarks(imgKey));
    const auto & landMarks = store->getLandMarks(imgKey);
    std::ofstream lmf(landmarksFilePath);
    lmf << landMarks->toJson(true);
    return landMarks;
}

void renderLandmarksOnImage(cv::Mat & image, const LandMarksSPtr & lm)
{
    using namespace cv;
    const Scalar detectionColor(250, 30, 0);
    rectangle(image, lm->vjFaceRect, Scalar(0, 128, 0), 2);
    rectangle(image, lm->vjLeftEyeRect, Scalar(0xA0, 0x52, 0x2D), 3);
    rectangle(image, lm->vjRightEyeRect, Scalar(0xA0, 0x52, 0x2D), 3);

    polylines(image, std::vector<std::vector<Point>> { lm->lipContour1st, lm->lipContour2nd }, true, detectionColor);
    rectangle(image, lm->vjMouthRect, Scalar(0xA0, 0x52, 0x2D), 3);

    circle(image, lm->eyeLeftPupil, 5, detectionColor, 2);
    circle(image, lm->eyeRightPupil, 5, detectionColor, 2);

    circle(image, lm->eyeLeftCorner, 5, detectionColor, 2);
    circle(image, lm->eyeRightCorner, 5, detectionColor, 2);
    circle(image, lm->noseTip, 5, detectionColor, 2);

    circle(image, lm->lipLeftCorner, 5, detectionColor, 2);
    circle(image, lm->lipRightCorner, 5, detectionColor, 2);

    circle(image, lm->crownPoint, 5, detectionColor, 2);
    circle(image, lm->chinPoint, 5, detectionColor, 2);

    int i = 0;
    for (const auto & pt : lm->allLandmarks) {
        cv::Point p(pt.x, pt.y);
        circle(image, p, 3, Scalar(40, 40, 190), 1);
        cv::putText(image, std::to_string(i++), p, cv::FONT_HERSHEY_DUPLEX, 0.4, CV_RGB(118, 185, 0), 1);
    }
}

bool loadJson(const std::string & filePath, rapidjson::Document & d)
{

    std::ifstream fs(filePath);
    if (fs.good()) {
        const auto content = std::string(std::istreambuf_iterator<char>(fs), std::istreambuf_iterator<char>());
        d.Parse(content.c_str());
        return true;
    }
    return false;
}

TEST(Research, ModelRatiosCalculations)
{
    const auto annCsvFile = resolvePath("research/mugshot_frontal_original_all/via_region_data_dpd.csv");
    const auto landMarksMap = importLandMarks(annCsvFile);

    std::vector<LandMarksSPtr> annotations;
    annotations.reserve(landMarksMap.size());
    for (const auto & kv : landMarksMap) {
        annotations.push_back(kv.second);
    }

    std::vector<double> c1;
    std::vector<double> c2;
    std::vector<double> crownEyeLineNormalizedDistances;
    for (const auto & lm : annotations) {
        auto frown = (lm->eyeLeftPupil + lm->eyeRightPupil) / 2.0;

        const auto refDist = norm(lm->eyeLeftPupil - lm->eyeRightPupil) + norm(frown - lm->chinPoint);

        const auto chinCrown = norm(lm->crownPoint - lm->chinPoint);
        const auto chinFrown = norm(frown - lm->chinPoint);

        c1.push_back(chinCrown / refDist);
        c2.push_back(chinFrown / refDist);

        const auto eyeLineCenter
            = Utilities::lineLineIntersection(lm->crownPoint, lm->chinPoint, lm->eyeLeftPupil, lm->eyeRightPupil);
        const auto frownDistance = norm(cv::Point2d(lm->crownPoint) - eyeLineCenter);

        crownEyeLineNormalizedDistances.push_back(frownDistance / chinCrown);
    }

    std::cout << "Chin-crown normalization: " << median(c1) << std::endl;
    std::cout << "Chin-frown normalization: " << median(c2) << std::endl;
    std::cout << "Crown-Eye distance / Chin-crown ratio: " << median(crownEyeLineNormalizedDistances) << std::endl;
}
} // namespace ppp

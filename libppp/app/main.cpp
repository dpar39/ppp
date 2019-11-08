#include <fstream>
#include <iostream>

#include <opencv2/imgcodecs.hpp>
#include <tclap/CmdLine.h>

#include "CanvasDefinition.h"
#include "IImageStore.h"
#include "LandMarks.h"

#include "PhotoDecorator.h"

#include "PhotoStandard.h"
#include "PppEngine.h"
#include <opencv2/imgproc/imgproc.hpp>

using namespace std;
using namespace ppp;

void configureEngine(const string & configFilePath, PppEngine & engine)
{
    // Read configuration file
    std::ifstream fs(configFilePath, std::ios_base::in);
    const std::string configString((std::istreambuf_iterator<char>(fs)), std::istreambuf_iterator<char>());
    if (!engine.configure(configString))
    {
        std::cerr << "Unable to load engine configuration. Exiting " << std::endl;
        std::terminate();
    }
}

std::string setImage(const string & inputImagePath, const PppEngine & engine)
{
    return engine.getImageStore()->setImage(inputImagePath);
}

void checkLicense()
{
    return; // This is FREE software no license check :)
    const auto today = std::chrono::system_clock::now();

    const auto stringToTime = [](const std::string & str) {
        using namespace std;
        using namespace std::chrono;

        int yyyy, mm, dd, hh, MM, ss, fff;
        const char scanfFormat[] = "%4d.%2d.%2d-%2d.%2d.%2d.%3d";
        sscanf(str.c_str(), scanfFormat, &yyyy, &mm, &dd, &hh, &MM, &ss, &fff);

        auto ttm = tm();
        ttm.tm_year = yyyy - 1900; // Year since 1900
        ttm.tm_mon = mm - 1; // Month since January
        ttm.tm_mday = dd; // Day of the month [1-31]
        ttm.tm_hour = hh; // Hour of the day [00-23]
        ttm.tm_min = MM;
        ttm.tm_sec = ss;

        const auto ttimeT = mktime(&ttm);

        auto timePointResult = system_clock::from_time_t(ttimeT);
        timePointResult += milliseconds(fff);
        return timePointResult;
    };

    const auto expiryTime = stringToTime("2019.03.08-00.00.00.000");
    if (today > expiryTime)
    {
        cout << "Hope you enjoyed this software. License has expired now. Please contact dpar39@gmail.com for more "
                "information"
             << std::endl;
        std::terminate();
    }
}

std::string today()
{
    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d");
    return oss.str();
}

void createEmployeeBadge(std::string & employeeName,
                         const std::string & employeeNumber,
                         const std::string & badgeDate,
                         const cv::Mat & croppedImage,
                         const std::string & badgeOutputFile)
{

    // Create badge
    const auto headerHeight = 80;
    const auto badgeWidthPx = croppedImage.size[0];
    const auto badgeHeightPx = croppedImage.size[1] + headerHeight * 2;

    cv::Mat badgeImage(badgeHeightPx, badgeWidthPx, croppedImage.type(), cv::Scalar(0));

    const auto heightPx = croppedImage.size[1];
    croppedImage.copyTo(badgeImage(cv::Rect(0, headerHeight, badgeWidthPx, heightPx)));

    const auto headerRect = cv::Rect(0, 0, badgeWidthPx, headerHeight);
    const auto footerRect = cv::Rect(0, badgeHeightPx - headerHeight, badgeWidthPx, headerHeight);

    const auto color = cv::Scalar(255, 255, 255);
    PhotoDecorator::drawText(badgeDate,
                             headerRect,
                             HorizontalAlignment::LEFT,
                             VerticalAlignment::CENTER,
                             badgeImage,
                             cv::FONT_HERSHEY_SIMPLEX,
                             1.2,
                             color,
                             2);

    PhotoDecorator::drawText(employeeNumber,
                             headerRect,
                             HorizontalAlignment::RIGHT,
                             VerticalAlignment::CENTER,
                             badgeImage,
                             cv::FONT_HERSHEY_SIMPLEX,
                             1.2,
                             color,
                             2);

    PhotoDecorator::drawText(employeeName,
                             footerRect,
                             HorizontalAlignment::LEFT,
                             VerticalAlignment::CENTER,
                             badgeImage,
                             cv::FONT_HERSHEY_SIMPLEX,
                             1.2,
                             color,
                             2);

    imwrite(badgeOutputFile, badgeImage, { cv::IMWRITE_PNG_COMPRESSION, 9 });
}

int main(int argc, char ** argv)
{
    checkLicense();

    const std::string dimensionTypeDescription = "dimension in mm | cm | inch";
    TCLAP::CmdLine cmd("Employee badge generation program", ' ', "1.0");

    // Input and output image file
    TCLAP::ValueArg<std::string> inputImage("i", "image", "Input photo taken by a camera", true, "", "file path");

    TCLAP::ValueArg<std::string> outPhotoImagePath("o",
                                                   "outImagePath",
                                                   "Output file containing the photo cropped to the passport standards",
                                                   true,
                                                   "",
                                                   "PNG image file path");
    TCLAP::ValueArg<std::string> outBadgeImagePath("b",
                                                   "outBadgePath",
                                                   "Output file containing the employee badge photo",
                                                   true,
                                                   "",
                                                   "PNG image file path");

    cmd.add(inputImage);
    cmd.add(outPhotoImagePath);
    cmd.add(outBadgeImagePath);

    // Configuration file (config.bundle.json) for Computer Vision engine
    TCLAP::ValueArg<std::string> configFile("",
                                            "config",
                                            "Configuration file with Computer Vision models",
                                            false,
                                            "config.bundle.json",
                                            "file path");
    cmd.add(configFile);

    // Photo standard arguments:
    TCLAP::ValueArg<double> photoWidth("", "photoWidth", "Photo standard: width", false, 2.0, dimensionTypeDescription);
    TCLAP::ValueArg<double> photoHeight("",
                                        "photoHeight",
                                        "Photo standard: height",
                                        false,
                                        2.0,
                                        dimensionTypeDescription);
    TCLAP::ValueArg<double> faceHeight("",
                                       "faceHeight",
                                       "Photo standard: distance from chin to top of the head",
                                       false,
                                       1.29,
                                       dimensionTypeDescription);
    TCLAP::ValueArg<double> bottomEyeLine("",
                                          "bottomEyeLine",
                                          "Photo standard: distance from bottom of photo to the eye line",
                                          false,
                                          1.18,
                                          dimensionTypeDescription);
    TCLAP::ValueArg<std::string> photoUnits("",
                                            "photoUnits",
                                            "Units for the photo size",
                                            false,
                                            "inch",
                                            "[mm | cm | inch]");
    cmd.add(photoWidth);
    cmd.add(photoHeight);
    cmd.add(faceHeight);
    cmd.add(bottomEyeLine);
    cmd.add(photoUnits);

    // Print definition arguments:
    TCLAP::ValueArg<double> printWidth("", "printWidth", "Print definition: width", false, 2.0, dimensionTypeDescription);
    TCLAP::ValueArg<double> printHeight("",
                                        "printHeight",
                                        "Print definition: height",
                                        false,
                                        2.0,
                                        dimensionTypeDescription);
    TCLAP::ValueArg<std::string> printUnits("",
                                            "printUnits",
                                            "Print definition: size dimension units",
                                            false,
                                            "inch",
                                            dimensionTypeDescription);
    TCLAP::ValueArg<double> printResolution("",
                                            "printResolution",
                                            "Print definition: print resolution",
                                            false,
                                            300.0,
                                            "Dots per Inch (DPI)");
    cmd.add(printWidth);
    cmd.add(printHeight);
    cmd.add(printUnits);
    cmd.add(printResolution);

    // Employee badge data
    TCLAP::ValueArg<std::string> employeeName("", "name", "Employee name", false, "CITIZEN, John", "Employee name");
    TCLAP::ValueArg<std::string> employeeNumber("", "number", "Employee number", false, "142", "Employee number");
    TCLAP::ValueArg<std::string> badgeDate("", "date", "Badge creation date", false, today(), "YYYY-MM-DD");

    cmd.add(employeeName);
    cmd.add(employeeNumber);
    cmd.add(badgeDate);

    try
    {
        cmd.parse(argc, argv);
    }
    catch (TCLAP::ArgException & e) // catch any exceptions
    {
        std::cerr << "Error parsing arguments: " << e.error() << " for arg " << e.argId() << std::endl;
        exit(0);
    }

    PppEngine engine;
    configureEngine(configFile.getValue(), engine);

    const auto imgKey = setImage(inputImage.getValue(), engine);

    LandMarks landmarks;
    engine.detectLandMarks(imgKey, landmarks);

    PhotoStandard ps(photoWidth.getValue(),
                     photoHeight.getValue(),
                     faceHeight.getValue(),
                     bottomEyeLine.getValue(),
                     photoUnits.getValue());

    CanvasDefinition cd(printWidth.getValue(),
                        printHeight.getValue(),
                        printResolution.getValue(),
                        printUnits.getValue());

    const auto output = engine.createTiledPrint(imgKey, ps, cd, landmarks.crownPoint, landmarks.chinPoint);

    // Save the cropped image
    imwrite(outPhotoImagePath.getValue(), output, { cv::IMWRITE_PNG_COMPRESSION, 9 });

    createEmployeeBadge(employeeName.getValue(),
                        employeeNumber.getValue(),
                        badgeDate.getValue(),
                        output,
                        outBadgeImagePath.getValue());
    return 0;
}

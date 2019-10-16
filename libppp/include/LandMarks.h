#pragma once

#include <opencv2/core/core.hpp>
#include <rapidjson/document.h>

struct LandMarks final
{
    std::string imageKey; ///<- Image ID that uniquely identifies the image being annotated
    // Eye Land marks
    cv::Point eyeLeftPupil; ///<- Position of the left eye pupil
    cv::Point eyeRightPupil; ///<- Position of the right eye pupil
    cv::Rect vjLeftEyeRect; ///<- Rectangle where the left eye was detected using Viola Jones algorithm
    cv::Rect vjRightEyeRect; ///<- Rectangle where the left eye was detected using Viola Jones algorithm

    int imageRotation; ///<- Possible values are 0, 90, -90, 180

    // Mouth marks
    cv::Point lipUpperCenter;
    cv::Point lipLowerCenter;
    cv::Point lipLeftCorner;
    cv::Point lipRightCorner;
    cv::Rect vjMouthRect;

    cv::Rect vjFaceRect;
    cv::Point crownPoint;
    cv::Point chinPoint;

    std::vector<cv::Point> lipContour1st;
    std::vector<cv::Point> lipContour2nd;

    cv::Point noseTip;
    cv::Point eyeLeftCorner;
    cv::Point eyeRightCorner;

    std::vector<cv::Point> allLandmarks;

    std::string toJson(bool prettyJson) const;
    void fromJson(const rapidjson::Value & v);
};

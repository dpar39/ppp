#pragma once
#include <IPhotoPrintMaker.h>

#include <gmock/gmock.h>

class MockPhotoPrintMaker : public IPhotoPrintMaker
{
public:
    MOCK_METHOD4(cropPicture, cv::Mat (const cv::Mat&, const cv::Point&, const cv::Point&, const PhotoStandard&));
    MOCK_METHOD3(tileCroppedPhoto, cv::Mat (const CanvasDefinition&, const PhotoStandard&, const cv::Mat&));
};

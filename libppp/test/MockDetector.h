#pragma once
#include <IDetector.h>

#include <gmock/gmock.h>

class MockDetector : public IDetector
{
public:
    MOCK_METHOD1(configure, void (rapidjson::Value&));

    MOCK_METHOD2(detectLandMarks, bool (const cv::Mat&, LandMarks&));
};
#pragma once
#include <IDetector.h>

#include <gmock/gmock.h>

namespace ppp
{
class MockDetector : public IDetector
{

private:
    void configureInternal(rapidjson::Value &) override
    {
        m_isConfigured = true;
    }

public:
    MOCK_METHOD2(detectLandMarks, bool(const cv::Mat &, LandMarks &));
};
} // namespace ppp

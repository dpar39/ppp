#pragma once
#include <IDetector.h>

#include <gmock/gmock.h>

namespace ppp
{
class MockDetector final : public IDetector
{

private:
    void configureInternal(const ConfigLoaderSPtr &) override
    {
        m_isConfigured = true;
    }

public:
    MOCK_METHOD2(detectLandMarks, bool(const cv::Mat &, LandMarks &));
};
} // namespace ppp

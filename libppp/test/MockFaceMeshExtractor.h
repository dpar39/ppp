#pragma once
#pragma once

#include "IFaceMeshExtractor.h"
#include <gmock/gmock.h>

namespace ppp {
class MockFaceMeshExtractor final : public IFaceMeshExtractor
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

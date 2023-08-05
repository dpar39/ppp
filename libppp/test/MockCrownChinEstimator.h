#pragma once

#include "ICrownChinEstimator.h"

#include <gmock/gmock.h>

namespace ppp {
class MockCrownChinEstimator : public ICrownChinEstimator
{
public:
    MOCK_METHOD1(configure, void(const ConfigLoaderSPtr &));
    MOCK_METHOD1(estimateCrownChin, bool(LandMarks &));
};
} // namespace ppp

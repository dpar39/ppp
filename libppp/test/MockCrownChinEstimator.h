#pragma once

#include "ICrownChinEstimator.h"

#include <gmock/gmock.h>

class MockCrownChinEstimator : public ICrownChinEstimator
{
public:
    MOCK_METHOD1(configure, void (rapidjson::Value&));
    MOCK_METHOD1(estimateCrownChin, bool (LandMarks&));
};
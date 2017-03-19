#pragma once

#include "ICrownChinEstimator.h"

FWD_DECL(CrownChinEstimator)

class CrownChinEstimator : public ICrownChinEstimator
{
public:
    void configure(rapidjson::Value& config) override;
    bool estimateCrownChin(LandMarks& landmarks) override;
};

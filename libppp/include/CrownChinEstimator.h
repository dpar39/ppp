#pragma once

#include "ICrownChinEstimator.h"

namespace ppp {
FWD_DECL(CrownChinEstimator)

class CrownChinEstimator final : public ICrownChinEstimator
{
public:
    void configure(const ConfigLoaderSPtr & config) override;

    bool estimateCrownChin(LandMarks & landMarks) override;

private:
    double m_chinCrownCoeff { 1.7699 };
    double m_chinFrownCoeff { 0.8945 };
};
} // namespace ppp

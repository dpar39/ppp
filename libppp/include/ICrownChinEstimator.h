#pragma once

#include <rapidjson/document.h>
#include "CommonHelpers.h"

struct LandMarks;

FWD_DECL(ICrownChinEstimator)

class ICrownChinEstimator : noncopyable
{
public:
    virtual ~ICrownChinEstimator() = default;

    /*!@brief Configures the detector from Json data !*/
    virtual void configure(rapidjson::Value &config) = 0;

    /*!@brief Estimate chin and crown point from the available landmarks!
    *  The result is written in the same LandMark structure !*/
    virtual bool estimateCrownChin(LandMarks &landmarks) = 0;
};

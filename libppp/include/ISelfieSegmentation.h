#pragma once

#include "CommonHelpers.h"
#include "IConfigurable.h"

#include <opencv2/core/core.hpp>

namespace ppp {

class ISelfieSegmentation : NonCopyable, public IConfigurable
{
public:
    /*!@brief Detects a subset of landmarks and stores them !
     *  @returns true if the intended landmarks were detected and can be used as input for subsequent detection, false
     * otherwise !*/
    virtual cv::Mat computeSegmentation(const cv::Mat & inputImage) = 0;
};
} // namespace ppp
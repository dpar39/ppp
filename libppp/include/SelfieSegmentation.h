#pragma once

#include "ISelfieSegmentation.h"

#include "opencv2/core/types.hpp"
#include <cstddef>

namespace mediapipe {
class CalculatorGraph;
} // namespace mediapipe

namespace ppp {

class SelfieSegmentation final : public ISelfieSegmentation
{
public:
    SelfieSegmentation() = default;
    ~SelfieSegmentation() override;
    cv::Mat computeSegmentation(const cv::Mat & inputImage) override;

private:
    void configureInternal(const ConfigLoaderSPtr & config) override;

    std::shared_ptr<mediapipe::CalculatorGraph> m_graph;

    size_t m_timeStamp;
    cv::Mat m_outputSegmentation {};
};
} // namespace ppp
#pragma once

#include "mediapipe/framework/port/status.h"
#include "opencv2/core/types.hpp"

namespace mediapipe {
class Packet;
class CalculatorGraph;
} // namespace mediapipe

namespace ppp {

namespace utils {
std::shared_ptr<mediapipe::CalculatorGraph> initializeGraph(
    const std::string & graphContent,
    const char * const outputStreamName,
    const std::function<absl::Status(const mediapipe::Packet &)> & outStreamCallback);

absl::Status addImageToInputStream(const cv::Mat & inputImage,
                                   size_t ts,
                                   const char * const inputStreamName,
                                   const std::shared_ptr<mediapipe::CalculatorGraph> & graph);

} // namespace utils
} // namespace ppp
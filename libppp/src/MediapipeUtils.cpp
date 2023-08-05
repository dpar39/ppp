#include "MediapipeUtils.h"

#include "mediapipe/framework/calculator_framework.h"
#include "mediapipe/framework/formats/image_frame.h"
#include "mediapipe/framework/formats/image_frame_opencv.h"
#include "mediapipe/framework/port/parse_text_proto.h"

#include <opencv2/imgproc/imgproc.hpp>
#include <string_view>

namespace ppp {
namespace utils {

std::shared_ptr<mediapipe::CalculatorGraph> initializeGraph(
    const std::string & graphContent,
    const char * const outputStreamName,
    const std::function<absl::Status(const mediapipe::Packet &)> & outStreamCallback)
{
    auto graph = std::make_shared<mediapipe::CalculatorGraph>();

    const auto status = [&]() -> absl::Status {
        auto proto = mediapipe::ParseTextProtoOrDie<mediapipe::CalculatorGraphConfig>(graphContent);
        VLOG(1) << "Initialize the calculator graph.";
        MP_RETURN_IF_ERROR(graph->Initialize(proto));

        VLOG(1) << "Start running the calculator graph.";
        graph->ObserveOutputStream(outputStreamName, outStreamCallback);

        MP_RETURN_IF_ERROR(graph->StartRun({}));
        return absl::OkStatus();
    }();

    if (!status.ok()) {
        assert(false && "failed to initialize mediapipe face mesh");
    }
    return graph;
}

absl::Status addImageToInputStream(const cv::Mat & inputImage,
                                   const size_t ts,
                                   const char * const inputStreamName,
                                   const std::shared_ptr<mediapipe::CalculatorGraph> & graph)
{
    cv::Mat cameraFrame;
    cv::cvtColor(inputImage, cameraFrame, cv::COLOR_BGR2RGB);
    // Wrap Mat into an ImageFrame.
    auto inputFrame = absl::make_unique<mediapipe::ImageFrame>(mediapipe::ImageFormat::SRGB,
                                                               cameraFrame.cols,
                                                               cameraFrame.rows,
                                                               mediapipe::ImageFrame::kDefaultAlignmentBoundary);
    cv::Mat inputFrameMat = mediapipe::formats::MatView(inputFrame.get());
    cameraFrame.copyTo(inputFrameMat);
    // Send image packet into the graph.
    MP_RETURN_IF_ERROR(
        graph->AddPacketToInputStream(inputStreamName,
                                      mediapipe::Adopt(inputFrame.release()).At(mediapipe::Timestamp(ts))));
    return absl::OkStatus();
}
} // namespace utils

} // namespace ppp
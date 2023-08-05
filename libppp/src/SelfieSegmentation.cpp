#include "SelfieSegmentation.h"
#include "ConfigLoader.h"
#include "IImageStore.h"
#include "LandMarks.h"
#include "MediapipeUtils.h"
#include "Utilities.h"

#include "mediapipe/framework/calculator_framework.h"
#include "mediapipe/framework/formats/image_frame.h"
#include "mediapipe/framework/formats/image_frame_opencv.h"
#include "mediapipe/framework/formats/landmark.pb.h"
#include "mediapipe/framework/formats/rect.pb.h"
#include "mediapipe/framework/port/file_helpers.h"
#include "mediapipe/framework/port/opencv_imgproc_inc.h"
#include "mediapipe/framework/port/parse_text_proto.h"
#include "mediapipe/framework/port/status.h"

#include <cstdlib>
#include <span>
#include <stdexcept>
#include <string>
#include <vector>

namespace ppp {
constexpr char K_INPUT_STREAM[] = "input_image"; // NOLINT (hicpp-avoid-c-arrays)
constexpr char K_OUTPUT_STREAM[] = "segmentation_mask"; // NOLINT (hicpp-avoid-c-arrays)

void SelfieSegmentation::configureInternal(const ConfigLoaderSPtr & /*config*/)
{
    std::string graphContent = R"(
        input_stream: "input_image"
        output_stream: "segmentation_mask"
        node {
            calculator: "SelfieSegmentationCpu"
            input_stream: "IMAGE:input_image"
            output_stream: "SEGMENTATION_MASK:segmentation_mask"
        }
    )";
    m_graph = utils::initializeGraph(graphContent, K_OUTPUT_STREAM, [&](const mediapipe::Packet & p) {
        const auto & outputSegmentationFrame = p.Get<mediapipe::ImageFrame>();
        m_outputSegmentation = mediapipe::formats::MatView(&outputSegmentationFrame).clone();
        return absl::OkStatus();
    });
    m_isConfigured = true;
}

SelfieSegmentation::~SelfieSegmentation()
{
    const auto shutdown = [this]() -> absl::Status {
        VLOG(1) << "Shutting down SelfieSegmentation";
        MP_RETURN_IF_ERROR(m_graph->CloseInputStream(K_INPUT_STREAM));
        return m_graph->WaitUntilDone();
    }();
}

cv::Mat SelfieSegmentation::computeSegmentation(const cv::Mat & inputImage)
{
    m_outputSegmentation = cv::Mat();
    const auto compute = [&]() -> absl::Status {
        utils::addImageToInputStream(inputImage, m_timeStamp++, K_INPUT_STREAM, m_graph);
        MP_RETURN_IF_ERROR(m_graph->WaitUntilIdle());
        return absl::OkStatus();
    }();
    return m_outputSegmentation;
}
} // namespace ppp
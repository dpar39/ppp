#include "FaceMeshExtractor.h"
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
#include "mediapipe/framework/port/status.h"

#include <cstdlib>
#include <span>
#include <stdexcept>
#include <string>
#include <vector>

namespace ppp {
constexpr char K_INPUT_STREAM[] = "input_video"; // NOLINT (hicpp-avoid-c-arrays)

void FaceMeshExtractor::configureInternal(const ConfigLoaderSPtr & /*config*/)
{
    std::string graphContent = R"(
        input_stream: "input_video"
        output_stream: "multi_face_landmarks"

        # Defines side packets for further use in the graph.
        node {
            calculator: "ConstantSidePacketCalculator"
            output_side_packet: "PACKET:0:num_faces"
            output_side_packet: "PACKET:1:with_attention"
            output_side_packet: "PACKET:2:use_prev_landmarks"
            node_options: {
                [type.googleapis.com/mediapipe.ConstantSidePacketCalculatorOptions]: {
                    packet { int_value: 1 }
                    packet { bool_value: true }
                    packet { bool_value: false }
                }
            }
        }

        # Subgraph that detects faces and corresponding landmarks.
        node {
            calculator: "FaceLandmarkFrontCpu"
            input_stream: "IMAGE:input_video"
            input_side_packet: "NUM_FACES:num_faces"
            input_side_packet: "WITH_ATTENTION:with_attention"
            input_side_packet: "USE_PREV_LANDMARKS:use_prev_landmarks"
            output_stream: "LANDMARKS:multi_face_landmarks"
            output_stream: "ROIS_FROM_LANDMARKS:face_rects_from_landmarks"
            output_stream: "DETECTIONS:face_detections"
            output_stream: "ROIS_FROM_DETECTIONS:face_rects_from_detections"
        }
    )";

    m_graph = utils::initializeGraph(graphContent, "multi_face_landmarks", [&](const mediapipe::Packet & p) {
        const auto & lmList = p.Get<std::vector<mediapipe::NormalizedLandmarkList>>();
        const auto & landmarksResult = lmList.front();
        m_landmarks.reserve(landmarksResult.landmark_size());
        for (auto i = 0; i < landmarksResult.landmark_size(); ++i) {
            const auto & lm = landmarksResult.landmark().at(i);
            m_landmarks.emplace_back(lm.x(), lm.y(), lm.z());
        }
        return absl::OkStatus();
    });
    m_isConfigured = true;
}

FaceMeshExtractor::~FaceMeshExtractor()
{
    const auto shutdown = [this]() -> absl::Status {
        VLOG(1) << "Shutting down FaceMeshExtractor";
        MP_RETURN_IF_ERROR(m_graph->CloseInputStream(K_INPUT_STREAM));
        return m_graph->WaitUntilDone();
    }();
}

cv::Point getLandMarkCenter(const std::vector<cv::Point3f> & landmarks, const std::span<const int> & indices)
{
    cv::Point3f result {};
    for (const auto & idx : indices) {
        result += landmarks[idx];
    }
    result = result / static_cast<int>(indices.size());
    return { roundInteger(result.x), roundInteger(result.y) };
}

bool FaceMeshExtractor::detectLandMarks(const cv::Mat & inputImage, LandMarks & landmarks)
{
    m_landmarks.clear();
    const auto compute = [&]() -> absl::Status {
        utils::addImageToInputStream(inputImage, m_timeStamp++, K_INPUT_STREAM, m_graph);

        MP_RETURN_IF_ERROR(m_graph->WaitUntilIdle());

        auto & allLandmarks = landmarks.allLandmarks;
        allLandmarks.reserve(m_landmarks.size());

        for (const auto & pt : m_landmarks) {
            allLandmarks.emplace_back(pt.x * inputImage.cols, pt.y * inputImage.rows, pt.z);
        }

        // landmarks.lipLeftCorner = getLandMark(lms, );
        // landmarks.lipRightCorner = getLandMark(lms, LandMarkType::MOUTH_CORNER_RIGHT);
        landmarks.eyeLeftPupil = getLandMarkCenter(allLandmarks, RIGHT_EYE_UPPER0);
        landmarks.eyeRightPupil = getLandMarkCenter(allLandmarks, LEFT_EYE_UPPER0);
        landmarks.chinPoint = getLandMarkCenter(allLandmarks, CHIN_POINT);
        landmarks.noseTip = getLandMarkCenter(allLandmarks, NOSE_TIP);
        // landmarks.eyeLeftCorner = getLandMark(lms, LandMarkType::EYE_OUTER_CORNER_LEFT);
        // landmarks.eyeRightCorner = getLandMark(lms, LandMarkType::EYE_OUTER_CORNER_RIGHT);
        return absl::OkStatus();
    }();
    return compute.ok();
}
} // namespace ppp
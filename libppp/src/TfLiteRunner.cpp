#include "TfLiteRunner.h"

#include "mediapipe/util/tflite/operations/landmarks_to_transform_matrix.h"
#include "mediapipe/util/tflite/operations/transform_landmarks.h"
#include "mediapipe/util/tflite/operations/transform_tensor_bilinear.h"
#include "mediapipe/util/tflite/operations/transpose_conv_bias.h"

#include "tensorflow/lite/kernels/register.h"
#include "tensorflow/lite/optional_debug_tools.h"

#include <cstring>
#include <opencv2/imgproc/imgproc.hpp>
#include <stdexcept>

#ifdef SELFIE_SEGMENTATION_MODEL_PATH
#include "incbin.h"
#define STR_DC_X(x) #x // NOLINT(cppcoreguidelines-macro-usage)
#define STR_DC(str) STR_DC_X(str) // NOLINT(cppcoreguidelines-macro-usage)
INCBIN(SELFIE_SEGMENTATION_MODEL, STR_DC(SELFIE_SEGMENTATION_MODEL_PATH)); // NOLINT(hicpp-no-assembler)
#endif

namespace ppp {

TfLiteRunner::TfLiteRunner(const std::string & modelPathOrContent)
{
    // Load model

    if (modelPathOrContent.empty()) {
#ifdef SELFIE_SEGMENTATION_MODEL_PATH
        model = tflite::FlatBufferModel::BuildFromBuffer(reinterpret_cast<const char *>(gSELFIE_SEGMENTATION_MODELData),
                                                         gSELFIE_SEGMENTATION_MODELSize);
#else
        throw std::runtime_error("Empty model");
#endif
    } else {
        if (modelPathOrContent.length() > 1024) {
            m_model = tflite::FlatBufferModel::BuildFromBuffer(modelPathOrContent.c_str(), modelPathOrContent.size());
        } else {
            m_model = tflite::FlatBufferModel::BuildFromFile(modelPathOrContent.c_str());
        }
    }

    assert(m_model != nullptr);

    // Build the interpreter with the InterpreterBuilder.
    // Note: all Interpreters should be built with the InterpreterBuilder,
    // which allocates memory for the Interpreter and does various set up
    // tasks so that the Interpreter can read the provided model.
    tflite::ops::builtin::BuiltinOpResolver resolver;
    tflite::InterpreterBuilder builder(*m_model, resolver);
    resolver.AddCustom("Convolution2DTransposeBias", mediapipe::tflite_operations::RegisterConvolution2DTransposeBias());
    resolver.AddCustom("Landmarks2TransformMatrix",
                       mediapipe::tflite_operations::RegisterLandmarksToTransformMatrixV1(),
                       1);
    resolver.AddCustom("Landmarks2TransformMatrix",
                       mediapipe::tflite_operations::RegisterLandmarksToTransformMatrixV2(),
                       2);
    resolver.AddCustom("TransformTensorBilinear", mediapipe::tflite_operations::RegisterTransformTensorBilinearV1(), 1);
    resolver.AddCustom("TransformTensorBilinear", mediapipe::tflite_operations::RegisterTransformTensorBilinearV2(), 2);
    resolver.AddCustom("TransformLandmarks", mediapipe::tflite_operations::RegisterTransformLandmarksV1(), 1);
    resolver.AddCustom("TransformLandmarks", mediapipe::tflite_operations::RegisterTransformLandmarksV2(), 2);

    builder(&m_interpreter);
    assert(m_interpreter != nullptr);

    // Allocate tensor buffers.
    assert(m_interpreter->AllocateTensors() == kTfLiteOk);

    auto * tensor = m_interpreter->input_tensor(0);
    m_inputTensorHeight = tensor->dims->data[1];
    m_inputTensorWidth = tensor->dims->data[2];

    if (false) {
        printf("=== Pre-invoke Interpreter State ===\n");
        tflite::PrintInterpreterState(m_interpreter.get());
    }
    // Fill input buffers
    // TODO(user): Insert code to fill input tensors.
    // Note: The buffer of the input tensor with index `i` of type T can
    // be accessed with `T* input = interpreter->typed_input_tensor<T>(i);`

    // Run inference
    assert(m_interpreter->Invoke() == kTfLiteOk);

    if (false) {
        printf("\n\n=== Post-invoke Interpreter State ===\n");
        tflite::PrintInterpreterState(m_interpreter.get());
    }
}

cv::Mat TfLiteRunner::scaleImageToTensor(const cv::Mat & image, const CropMethod scaleCropMethod)
{
    using namespace cv;
    cv::Mat dst;
    cv::Size tensorSize(m_inputTensorWidth, m_inputTensorHeight);
    if (image.cols * m_inputTensorHeight == m_inputTensorWidth * image.rows
        || scaleCropMethod == CropMethod::TWEAK_PIXEL_RATIO_TO_FIT) {

        cv::resize(image, dst, tensorSize, 0, 0, cv::INTER_AREA);

    } else {
        const std::array<Point2f, 3> srcPoints = { Point2f { image.cols / 2.f, image.rows / 2.f },
                                                   { 0.f, static_cast<float>(image.rows) },
                                                   { static_cast<float>(image.cols), 0.f } };

        const auto xRatio = static_cast<float>(m_inputTensorWidth) / image.cols;
        const auto yRatio = static_cast<float>(m_inputTensorHeight) / image.rows;
        auto ratio = 0.f;

        std::array<Point2f, 3> dstPoints;
        if (scaleCropMethod == CropMethod::KEEP_RATIO_AND_FIT) {
            ratio = xRatio < yRatio ? xRatio : yRatio;
        } else if (scaleCropMethod == CropMethod::KEEP_RATIO_AND_CROP) {
            ratio = xRatio > yRatio ? xRatio : yRatio;
        } else {
            throw std::logic_error("Unexpected crop method");
        }

        auto tform = getRotationMatrix2D(Point2f(image.cols / 2.f, image.rows / 2.f), 0, ratio);
        tform.at<double>(0, 2) += m_inputTensorWidth / 2.0 - (image.cols) / 2.0;
        tform.at<double>(1, 2) += m_inputTensorHeight / 2.f - (image.rows) / 2.0;

        cv::warpAffine(image, dst, tform, tensorSize);

        cv::cvtColor(dst, dst, cv::COLOR_BGR2RGB);
        cv::normalize(dst, dst, -1.0, 1.0, cv::NORM_MINMAX, CV_32FC3);
    }
    return dst;
}

void TfLiteRunner::runImage(const cv::Mat & inputImage)
{
    auto * f = m_interpreter->typed_input_tensor<float>(0);
    memcpy(f, inputImage.data, inputImage.elemSize() * inputImage.cols * inputImage.rows);
    assert(m_interpreter->Invoke() == kTfLiteOk);
}

float * TfLiteRunner::getOutputTensor(int index) const
{
    return m_interpreter->typed_output_tensor<float>(index);
}
float * TfLiteRunner::getInputTensor(int index) const
{
    return m_interpreter->typed_input_tensor<float>(index);
}
std::vector<int> TfLiteRunner::getOutputTensorDims(int index) const
{
    auto * t = m_interpreter->output_tensor(index);
    return { t->dims->data, t->dims->data + t->dims->size };
}
std::vector<int> TfLiteRunner::getInputTensorDims(int index) const
{
    auto * t = m_interpreter->input_tensor(index);
    return { t->dims->data, t->dims->data + t->dims->size };
}
} // namespace ppp
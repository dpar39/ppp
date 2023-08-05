#pragma once
#include <string>

#include "tensorflow/lite/model.h"

#include "opencv2/core/types.hpp"

namespace ppp {

enum class CropMethod
{
    KEEP_RATIO_AND_CROP,
    KEEP_RATIO_AND_FIT,
    TWEAK_PIXEL_RATIO_TO_FIT
};

class TfLiteRunner
{

public:
    explicit TfLiteRunner(const std::string & modelPathOrContent);

    cv::Mat scaleImageToTensor(const cv::Mat & image, CropMethod scaleCropMethod);

    void runImage(const cv::Mat & inputImage);

    float * getInputTensor(int index) const;

    float * getOutputTensor(int index) const;

    std::vector<int> getInputTensorDims(int index) const;

    std::vector<int> getOutputTensorDims(int index) const;

private:
    std::unique_ptr<tflite::FlatBufferModel> m_model;
    std::unique_ptr<tflite::Interpreter> m_interpreter;

    int m_inputTensorWidth;
    int m_inputTensorHeight;
};
} // namespace ppp
#ifdef __EMSCRIPTEN__

#include "PublicPppEngine.h"

#include <glog/logging.h>
#include <opencv2/core/types.hpp>

#include <emscripten.h>
#include <emscripten/bind.h>

using namespace emscripten;
using namespace cv;
namespace ppp {

template <typename T>
val matData(const cv::Mat & mat)
{
    return val(emscripten::memory_view<T>((mat.total() * mat.elemSize()) / sizeof(T), (T *)mat.data));
}
void InitGoogleLogging()
{
    ::google::InitGoogleLogging("wasm");
}

EMSCRIPTEN_BINDINGS(my_module)
{
    emscripten::function("InitGoogleLogging", &InitGoogleLogging);

    emscripten::class_<PublicPppEngine>("Engine")
        .function("configure", &PublicPppEngine::configure)
        .function("isConfigured", &PublicPppEngine::isConfigured)
        .function("setImage", &PublicPppEngine::setImage)
        .function("getImage", &PublicPppEngine::getImage)
        .function("detectLandmarks", &PublicPppEngine::detectLandmarks)
        .constructor<>();

    emscripten::class_<cv::Mat>("Mat")
        .constructor<>()
        .constructor<const Mat &>()
        .constructor<Size, int>()
        .constructor<int, int, int>()
        .constructor<int, int, int, const Scalar &>()
        .property("rows", &cv::Mat::rows)
        .property("cols", &cv::Mat::cols)
        .property("data", &matData<unsigned char>)
        .property("data8S", &matData<char>)
        .property("data16U", &matData<unsigned short>)
        .property("data16S", &matData<short>)
        .property("data32S", &matData<int>)
        .property("data32F", &matData<float>)
        .property("data64F", &matData<double>)
        .function("elemSize", select_overload<size_t() const>(&cv::Mat::elemSize))
        .function("elemSize1", select_overload<size_t() const>(&cv::Mat::elemSize1))
        .function("channels", select_overload<int() const>(&cv::Mat::channels))
        .function("total", select_overload<size_t() const>(&cv::Mat::total))
        .function("row", select_overload<Mat(int) const>(&cv::Mat::row))
        .function("create", select_overload<void(int, int, int)>(&cv::Mat::create))
        .function("create", select_overload<void(Size, int)>(&cv::Mat::create))
        .function("rowRange", select_overload<Mat(int, int) const>(&cv::Mat::rowRange))
        .function("rowRange", select_overload<Mat(const Range &) const>(&cv::Mat::rowRange))
        .function("type", select_overload<int() const>(&cv::Mat::type))
        .function("empty", select_overload<bool() const>(&cv::Mat::empty))
        .function("colRange", select_overload<Mat(int, int) const>(&cv::Mat::colRange))
        .function("colRange", select_overload<Mat(const Range &) const>(&cv::Mat::colRange))
        .function("step1", select_overload<size_t(int) const>(&cv::Mat::step1))
        .function("clone", select_overload<Mat() const>(&cv::Mat::clone))
        .function("depth", select_overload<int() const>(&cv::Mat::depth))
        .function("col", select_overload<Mat(int) const>(&cv::Mat::col))
        .function("roi", select_overload<Mat(const Rect &) const>(&cv::Mat::operator()))
        .function("isContinuous", select_overload<bool() const>(&cv::Mat::isContinuous))
        .function("charAt", select_overload<char &(int)>(&cv::Mat::at<char>))
        .function("charAt", select_overload<char &(int, int)>(&cv::Mat::at<char>))
        .function("charAt", select_overload<char &(int, int, int)>(&cv::Mat::at<char>))
        .function("ucharAt", select_overload<unsigned char &(int)>(&cv::Mat::at<unsigned char>))
        .function("ucharAt", select_overload<unsigned char &(int, int)>(&cv::Mat::at<unsigned char>))
        .function("ucharAt", select_overload<unsigned char &(int, int, int)>(&cv::Mat::at<unsigned char>))
        .function("shortAt", select_overload<short &(int)>(&cv::Mat::at<short>))
        .function("shortAt", select_overload<short &(int, int)>(&cv::Mat::at<short>))
        .function("shortAt", select_overload<short &(int, int, int)>(&cv::Mat::at<short>))
        .function("ushortAt", select_overload<unsigned short &(int)>(&cv::Mat::at<unsigned short>))
        .function("ushortAt", select_overload<unsigned short &(int, int)>(&cv::Mat::at<unsigned short>))
        .function("ushortAt", select_overload<unsigned short &(int, int, int)>(&cv::Mat::at<unsigned short>))
        .function("intAt", select_overload<int &(int)>(&cv::Mat::at<int>))
        .function("intAt", select_overload<int &(int, int)>(&cv::Mat::at<int>))
        .function("intAt", select_overload<int &(int, int, int)>(&cv::Mat::at<int>))
        .function("floatAt", select_overload<float &(int)>(&cv::Mat::at<float>))
        .function("floatAt", select_overload<float &(int, int)>(&cv::Mat::at<float>))
        .function("floatAt", select_overload<float &(int, int, int)>(&cv::Mat::at<float>))
        .function("doubleAt", select_overload<double &(int, int, int)>(&cv::Mat::at<double>))
        .function("doubleAt", select_overload<double &(int)>(&cv::Mat::at<double>))
        .function("doubleAt", select_overload<double &(int, int)>(&cv::Mat::at<double>));
}
} // namespace ppp
#endif
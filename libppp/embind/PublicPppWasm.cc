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
}
} // namespace ppp
#endif
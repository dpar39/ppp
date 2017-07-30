#include "NativeWrapper.h"
#include "NativeLog.h"

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <libppp.h>


std::shared_ptr<PublicPppEngine> NativeWrapper::s_enginePtr = std::make_shared<PublicPppEngine>();

void NativeWrapper::configure(const char *jsonConfig)
{
    s_enginePtr->configure(std::string(jsonConfig));
}

const char * NativeWrapper::setImage(const char *imgBase64)
{
    return "setImageCalled";
}

const char * NativeWrapper::detectLandMarks(const char *imgKey)
{
    return "detectLandMarks";
}

const char * NativeWrapper::createTiledPrint(const char *imgKey, const char *request)
{
    return "createTiledPrint";
}
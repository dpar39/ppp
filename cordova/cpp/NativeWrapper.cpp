#include "NativeWrapper.h"
#include "NativeLog.h"

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <libppp.h>

std::shared_ptr<PublicPppEngine> NativeWrapper::s_enginePtr = std::make_shared<PublicPppEngine>();

const char * NativeWrapper::configure(const char *jsonConfig)
{
    try
    {
        bool success = s_enginePtr->configure(jsonConfig);
        return success ? "SUCCESS!" : "FAILURE";
    }
    catch (...)
    {
        return "ERROR";
    }
}

const char * NativeWrapper::setImage(const char *imgBase64)
{
    try
    {
        auto imgKey = s_enginePtr->setImage(imgBase64, 0);
        return imgKey.c_str();
    }
    catch(const std::exception &e)
    {
        return e.what();
    }
}

const char * NativeWrapper::detectLandMarks(const char *imgKey)
{
    try
    {
        auto landmarksJson = s_enginePtr->detectLandmarks(std::string(imgKey));
        return landmarksJson.c_str();
    }
    catch(const std::exception &e)
    {
        return e.what();
    }
}

const char * NativeWrapper::createTiledPrint(const char *imgKey, const char *request)
{
    return "createTiledPrint not implemented!";
}
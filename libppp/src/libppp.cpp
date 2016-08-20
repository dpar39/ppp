#include "libppp.h"
#include "PppEngine.h"
#include "LandMarks.h"
#include "PhotoStandard.h"
#include "CanvasDefinition.h"

#include <opencv2/imgcodecs.hpp>

using namespace rapidjson;

cv::Point fromJson(Value& v)
{
    /*if (v.Empty() || v["x"].Empty() || v["y"].Empty())
    {
        return cv::Point(0, 0);
    }*/
    return cv::Point(v["x"].GetInt(), v["y"].GetInt());
}

PublicPppEngine::PublicPppEngine()
{
    m_pPppEngine = std::make_shared<PppEngine>();
}

void PublicPppEngine::configure(const std::string& jsonConfig)
{
    Document parser;
    parser.Parse(jsonConfig.c_str());
    m_pPppEngine->configure(parser);
}

std::string PublicPppEngine::setImage(const char* bufferData, int bufferLength)
{
    cv::_InputArray inputArray(bufferData, bufferLength);
    auto inputImage = cv::imdecode(inputArray, cv::IMREAD_COLOR);
    return m_pPppEngine->setInputImage(inputImage);
}

std::string PublicPppEngine::detectLandmarks(const std::string& imageId)
{
    LandMarks landMarks;
    m_pPppEngine->detectLandMarks(imageId, landMarks);
    return landMarks.toJson();
}

void PublicPppEngine::createTiledPrint(const std::string& imageId, const std::string& request, std::vector<byte>& pictureData)
{
    rapidjson::Document d;
    d.Parse(request.c_str());

    auto ps = PhotoStandard::fromJson(d["standard"]);
    auto canvas = CanvasDefinition::fromJson(d["canvas"]);
    auto cronwPoint = fromJson(d["crownPoint"]);
    auto chinPoint = fromJson(d["chinPoint"]);

    auto result = m_pPppEngine->createTiledPrint(imageId, *ps, *canvas, cronwPoint, chinPoint);

    cv::imencode(".png", result, pictureData);
}

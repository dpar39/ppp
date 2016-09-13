#include "libppp.h"
#include "PppEngine.h"
#include "LandMarks.h"
#include "PhotoStandard.h"
#include "CanvasDefinition.h"
#include "Geometry.h"

#include <opencv2/imgcodecs.hpp>

using namespace rapidjson;
using namespace std;

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

    // Add image resolution to output

    setPngResolutionDpi(pictureData, canvas->resolutionPixelsPerMM());

}

template <typename T>
std::vector<byte> toBytes(const T& x)
{
     vector<byte> v(static_cast<const byte*>(static_cast<const void*>(&x)),
        static_cast<const byte*>(static_cast<const void*>(&x)) + sizeof(x));
     reverse(v.begin(), v.end());
     return v;
}

/*  The pHYs chunk specifies the intended pixel size or aspect ratio for display of the image. It contains:
    Pixels per unit, X axis: 4 bytes (unsigned integer)
    Pixels per unit, Y axis: 4 bytes (unsigned integer)
    Unit specifier:          1 byte
The following values are defined for the unit specifier:
    0: unit is unknown
    1: unit is the meter
pHYs has to go before IDAT chunk
*/
void PublicPppEngine::setPngResolutionDpi(std::vector<byte> &imageStream, double resolution_ppmm)
{
    auto chunkLenBytes = toBytes(9);
    auto resolBytes = toBytes(ROUND_INT(resolution_ppmm * 1000));
    string physStr = "pHYs";

    auto pHYsChunk(chunkLenBytes);
    pHYsChunk.insert(pHYsChunk.end(), physStr.begin(), physStr.end());

    pHYsChunk.insert(pHYsChunk.end(), resolBytes.begin(), resolBytes.end());
    pHYsChunk.insert(pHYsChunk.end(), resolBytes.begin(), resolBytes.end());
    pHYsChunk.push_back(1); // Unit is the meter

    auto crcBytes = toBytes(update_crc(0, &pHYsChunk[4], pHYsChunk.size() - 4));
    pHYsChunk.insert(pHYsChunk.end(), crcBytes.begin(), crcBytes.end());

    string idat = "IDAT";
    auto it = search(imageStream.begin(), imageStream.end(), idat.begin(), idat.end());

    if (it != imageStream.end())
    {
        // Insert the chunk in the stream
        imageStream.insert(it - 4, pHYsChunk.begin(), pHYsChunk.end());
    }
}

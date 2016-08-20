#include "LandMarks.h"

#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>


rapidjson::Value pointToJson(const cv::Point& p, rapidjson::Document::AllocatorType& alloc)
{
    rapidjson::Value obj(rapidjson::kObjectType);
    obj.AddMember("x", p.x, alloc);
    obj.AddMember("y", p.y, alloc);
    return obj;
}

rapidjson::Value rectangleToJson(const cv::Rect& r, rapidjson::Document::AllocatorType& alloc)
{
    rapidjson::Value obj(rapidjson::kObjectType);
    obj.AddMember("x", r.x, alloc);
    obj.AddMember("y", r.y, alloc);
    obj.AddMember("width", r.width, alloc);
    obj.AddMember("height", r.height, alloc);
    return obj;
}


std::string LandMarks::toString() const
{
    std::stringstream ss;
    ss << eyeLeftPupil << eyeRightPupil << vjLeftEyeRect << vjRightEyeRect;

    return ss.str();
}

std::string LandMarks::toJson() const
{
    using namespace rapidjson;
    Document d;
    d.SetObject();
    auto& alloc = d.GetAllocator();
    d.AddMember("vjFaceRect", rectangleToJson(vjFaceRect, alloc), alloc);

    d.AddMember("eyeLeftPupil", pointToJson(eyeLeftPupil, alloc), alloc);
    d.AddMember("eyeRightPupil", pointToJson(eyeRightPupil, alloc), alloc);
    d.AddMember("vjLeftEyeRect", rectangleToJson(vjLeftEyeRect, alloc), alloc);
    d.AddMember("vjRightEyeRect", rectangleToJson(vjRightEyeRect, alloc), alloc);

    d.AddMember("lipUpperCenter", pointToJson(lipUpperCenter, alloc), alloc);
    d.AddMember("lipLowerCenter", pointToJson(lipLowerCenter, alloc), alloc);
    d.AddMember("lipLeftCorner", pointToJson(lipLeftCorner, alloc), alloc);
    d.AddMember("lipRightCorner", pointToJson(lipRightCorner, alloc), alloc);

    d.AddMember("crownPoint", pointToJson(crownPoint, alloc), alloc);
    d.AddMember("chinPoint", pointToJson(chinPoint, alloc), alloc);

    StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);
    d.Accept(writer);

    return std::string(buffer.GetString());
}

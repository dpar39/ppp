#include "LandMarks.h"
#include "Utilities.h"
#include <rapidjson/document.h>

namespace ppp
{

rapidjson::Value pointToJson(const cv::Point & p, rapidjson::Document::AllocatorType & alloc)
{
    rapidjson::Value obj(rapidjson::kObjectType);
    obj.AddMember("x", p.x, alloc);
    obj.AddMember("y", p.y, alloc);
    return obj;
}

rapidjson::Value rectToJson(const cv::Rect & r, rapidjson::Document::AllocatorType & alloc)
{
    rapidjson::Value obj(rapidjson::kObjectType);
    obj.AddMember("x", r.x, alloc);
    obj.AddMember("y", r.y, alloc);
    obj.AddMember("width", r.width, alloc);
    obj.AddMember("height", r.height, alloc);
    return obj;
}

void jsonToPoint(const rapidjson::Value & v, cv::Point & pt)
{
    pt.x = v["x"].GetFloat();
    pt.y = v["y"].GetFloat();
}

void jsonToRect(const rapidjson::Value & v, cv::Rect & r)
{
    r.x = v["x"].GetFloat();
    r.y = v["y"].GetFloat();
    r.width = v["width"].GetFloat();
    r.height = v["height"].GetFloat();
}

std::string LandMarks::toJson(const bool prettyJson) const
{
    using namespace rapidjson;
    Document d;
    d.SetObject();
    auto & alloc = d.GetAllocator();

#define SERIALIZE_POINT(pt)                                                                                            \
    if (pt.x != 0 || pt.y != 0)                                                                                        \
        d.AddMember(#pt, pointToJson(pt, alloc), alloc);

#define SERIALIZE_RECT(r)                                                                                              \
    if (r.x != 0 || r.y != 0 || r.width != 0 || r.height != 0)                                                         \
        d.AddMember(#r, rectToJson(r, alloc), alloc);

    SERIALIZE_RECT(vjFaceRect);
    SERIALIZE_RECT(vjLeftEyeRect);
    SERIALIZE_RECT(vjRightEyeRect);
    SERIALIZE_RECT(vjMouthRect);

    SERIALIZE_POINT(crownPoint);
    SERIALIZE_POINT(chinPoint);

    SERIALIZE_POINT(noseTip);
    SERIALIZE_POINT(eyeLeftPupil);
    SERIALIZE_POINT(eyeRightPupil);
    SERIALIZE_POINT(eyeLeftCorner);
    SERIALIZE_POINT(eyeRightCorner);

    SERIALIZE_POINT(lipUpperCenter);
    SERIALIZE_POINT(lipLowerCenter);
    SERIALIZE_POINT(lipLeftCorner);
    SERIALIZE_POINT(lipRightCorner);
    return Utilities::serializeJson(d, prettyJson);
}

void LandMarks::fromJson(const rapidjson::Value & v)
{
#define PARSE_POINT(pt)                                                                                                \
    if (v.HasMember(#pt))                                                                                              \
        jsonToPoint(v[#pt], pt);

#define PARSE_RECT(r)                                                                                                  \
    if (v.HasMember(#r))                                                                                               \
        jsonToRect(v[#r], r);

    PARSE_RECT(vjFaceRect);
    PARSE_RECT(vjLeftEyeRect);
    PARSE_RECT(vjRightEyeRect);
    PARSE_RECT(vjMouthRect);

    PARSE_POINT(crownPoint);
    PARSE_POINT(chinPoint);

    PARSE_POINT(noseTip);

    PARSE_POINT(eyeLeftPupil);
    PARSE_POINT(eyeRightPupil);
    PARSE_POINT(eyeLeftCorner);
    PARSE_POINT(eyeRightCorner);

    PARSE_POINT(lipUpperCenter);
    PARSE_POINT(lipLowerCenter);
    PARSE_POINT(lipLeftCorner);
    PARSE_POINT(lipRightCorner);
    PARSE_POINT(crownPoint);
    PARSE_POINT(chinPoint);
}

LandMarksSPtr LandMarks::create()
{
    return std::make_shared<LandMarks>();
}
} // namespace ppp

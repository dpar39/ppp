#include "LandMarks.h"

#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>

#include <FaceDetector.h>


rapidjson::Value pointToJson(const cv::Point& p, rapidjson::Document& d)
{
    rapidjson::Value obj;
    obj.AddMember("x", p.x, d.GetAllocator());
    obj.AddMember("y", p.y, d.GetAllocator());
    return obj;
}

rapidjson::Value rectangleToJson(const cv::Rect& r, rapidjson::Document& d)
{
    rapidjson::Value obj;
    obj.AddMember("x", r.x, d.GetAllocator());
    obj.AddMember("y", r.y, d.GetAllocator());
    obj.AddMember("width", r.width, d.GetAllocator());
    obj.AddMember("height", r.height, d.GetAllocator());
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
    Value& obj = d.SetObject();

    d["vjFaceRect"] = rectangleToJson(vjFaceRect, d);

    d["eyeLeftPupil"] = pointToJson(eyeLeftPupil, d);
    d["eyeRightPupil"] = pointToJson(eyeRightPupil, d);
    d["vjLeftEyeRect"] = rectangleToJson(vjLeftEyeRect, d);
    d["vjRightEyeRect"] = rectangleToJson(vjRightEyeRect, d);

    d["lipUpperCenter"] = pointToJson(lipUpperCenter, d);
    d["lipLowerCenter"] = pointToJson(lipLowerCenter, d);
    d["lipLeftCorner"] = pointToJson(lipLeftCorner, d);
    d["lipRightCorner"] = pointToJson(lipRightCorner, d);

    d["crownPoint"] = pointToJson(crownPoint, d);
    d["chinPoint"] = pointToJson(chinPoint, d);

    StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);
    d.Accept(writer);

    return std::string(buffer.GetString());
}

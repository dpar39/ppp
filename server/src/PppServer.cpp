//
// Created by Darien Pardinas Diaz on 9/25/18.
//
#include "PppServer.h"
#include <rapidjson/rapidjson.h>
#include <rapidjson/document.h>

#include <fstream>

void PppServer::init()
{
    using namespace rapidjson;

    std::ifstream configFile("config.json");
    const auto configStr = std::string(std::istreambuf_iterator<char>(configFile), std::istreambuf_iterator<char>());
    _pppEngine.configure(configStr.c_str());

    addRoute(RouteDefinition::create("/api/upload", { http::verb::get }, [this](const Request & req) {
        MultiPartFormParser parser;
        const auto success = parser.parse(req, req.body());

        if (success)
        {
            const auto & body = req.body();
            auto fileContent = body.c_str() + parser.contentStartOffset();
            auto fileSize = parser.contentSize();
            auto imgKey = _pppEngine.setImage(fileContent, fileSize);

//            Document d;
//            d.SetObject();
//            auto & alloc = d.GetAllocator();
//
//            d.AddMember("imgKey", imgKey, alloc);

//            res.write(crow::json::dump(x));
//            res.end();
            return;
        }
//        res.write("Failed to upload image");
//        res.code = 500;
//        res.end();
    }));

    addRoute(RouteDefinition::create("/api/landmarks", { http::verb::get }, [this](const Request & req) {

        auto imgKey = "";
        //auto imgKey = req.url_params.get("imgKey");

        // TODO: Check if imgKey exists first

        const auto landmarks = _pppEngine.detectLandmarks(imgKey);

//        res.write(landmarks);
//        res.end();
    }));
}

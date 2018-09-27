#include <crow_all.h>

#include "CrowServer.h"

#include "MultiPartFormParser.h"

#include "libppp.h"


void StaticFileServer::serveFile(const std::string & url, crow::response & res)
{
    const auto fullPath = m_prefix + "/" + url;

    std::ifstream fs(fullPath, std::ifstream::in);
    if (fs)
    {
        const auto fileContent = std::string(std::istreambuf_iterator<char>(fs), std::istreambuf_iterator<char>());
        const auto contentType = getMimeType(fullPath);
        res.add_header("Content-Type", contentType);
        res.write(fileContent);
        res.end();
        return;
    }
}

std::string StaticFileServer::getMimeType(const std::string & filePath)
{
    auto const pos = filePath.rfind(".");
    if (pos == std::string::npos)
        return "application/text";

    const auto str = filePath.substr(pos);

    static std::unordered_map<std::string, std::string> sKnownMimeTypes = { { "htm", "text/html" },
                                                                            { ".html", "text/html" },
                                                                            { ".css", "text/css" },
                                                                            { ".txt", "text/plain" },
                                                                            { ".js", "application/javascript" },
                                                                            { ".json", "application/json" },
                                                                            { ".xml", "application/xml" },
                                                                            { ".swf",
                                                                                    "application/x-shockwave-flash" },
                                                                            { ".flv", "video/x-flv" },
                                                                            { ".png", "image/png" },
                                                                            { ".jpe", "image/jpeg" },
                                                                            { ".jpeg", "image/jpeg" },
                                                                            { ".jpg", "image/jpeg" },
                                                                            { ".gif", "image/gif" },
                                                                            { ".bmp", "image/bmp" },
                                                                            { ".ico", "image/vnd.microsoft.icon" },
                                                                            { ".tiff", "image/tiff" },
                                                                            { ".tif", "image/tiff" },
                                                                            { ".svg", "image/svg+xml" },
                                                                            { ".svgz", "image/svg+xml" } };
    const auto it = sKnownMimeTypes.find(str);
    if (it != sKnownMimeTypes.end())
    {
        return it->second;
    }
    return "application/text";
}

struct ExampleMiddleware
{
    StaticFileServer sfs;

    ExampleMiddleware()
            : sfs("./dist")
    {
    }

    struct context
    {
    };

    void before_handle(crow::request & req, crow::response & res, context & /*ctx*/)
    {
        CROW_LOG_INFO << ": " << req.url;

        if (req.method == crow::HTTPMethod::Get)
        {
            auto url = req.url;
            if (url == "/")
            {
                sfs.serveFile("index.html", res);
            }
            else
            {
                sfs.serveFile(url, res);
            }
        }
    }

    void after_handle(crow::request & req, crow::response & res, context & /*ctx*/)
    {
        // no-op
    }
};


void CrowServer::run(uint16_t port)
{
    crow::App<ExampleMiddleware> app;

    PublicPppEngine pppEngine;

    std::ifstream configFile("config.json");
    const auto configStr = std::string(std::istreambuf_iterator<char>(configFile), std::istreambuf_iterator<char>());
    pppEngine.configure(configStr.c_str());

    CROW_ROUTE(app, "/api/upload").methods("POST"_method)([&pppEngine](const crow::request & req, crow::response & res) {
        MultiPartFormParser parser;
        const auto success = parser.parse(req.headers, req.body);
        crow::json::wvalue x;
        if (success)
        {
            const auto & body = req.body;
            auto fileContent = body.c_str() + parser.contentStartOffset();
            auto fileSize = parser.contentSize();
            const auto uid = pppEngine.setImage(fileContent, fileSize);
            crow::json::wvalue x;
            x["imgKey"] = uid;
            res.write(crow::json::dump(x));
            res.end();
            return;
        }
        res.write("Failed to upload image");
        res.code = 500;
        res.end();
    });

    CROW_ROUTE(app, "/api/landmarks")
            ([&pppEngine](const crow::request & req, crow::response & res) {
                auto imgKey = req.url_params.get("imgKey");

                // TODO: Check if imgKey exists first

                const auto landmarks = pppEngine.detectLandmarks(imgKey);
                res.write(landmarks);
                res.end();
            });

    //    // simple json response
    //    CROW_ROUTE(app, "/json")
    //    ([] {
    //        crow::json::wvalue x;
    //        x["message"] = "Hello, World!";
    //        return x;
    //    });
    //
    //    CROW_ROUTE(app, "/add/<int>/<int>")
    //    ([](const crow::request & /*req*/, crow::response & res, int a, int b) {
    //        std::ostringstream os;
    //        os << a + b;
    //        res.write(os.str());
    //        res.end();
    //    });
    //
    // Compile error with message "Handler type is mismatched with URL paramters"
    // CROW_ROUTE(app,"/another/<int>")
    //([](int a, int b){
    // return crow::response(500);
    //});
    //
    // more json example
    //    CROW_ROUTE(app, "/add_json")
    //    ([](const crow::request & req) {
    //        auto x = crow::json::load(req.body);
    //        if (!x)
    //            return crow::response(400);
    //        int sum = x["a"].i() + x["b"].i();
    //        std::ostringstream os;
    //        os << sum;
    //        return crow::response { os.str() };
    //    });
    //
    //    CROW_ROUTE(app, "/params")
    //    ([](const crow::request & req) {
    //        std::ostringstream os;
    //        os << "Params: " << req.url_params << "\n\n";
    //        os << "The key 'foo' was " << (req.url_params.get("foo") == nullptr ? "not " : "") << "found.\n";
    //        if (req.url_params.get("pew") != nullptr)
    //        {
    //            double countD = boost::lexical_cast<double>(req.url_params.get("pew"));
    //            os << "The value of 'pew' is " << countD << '\n';
    //        }
    //        auto count = req.url_params.get_list("count");
    //        os << "The key 'count' contains " << count.size() << " value(s).\n";
    //        for (const auto & countVal : count)
    //        {
    //            os << " - " << countVal << '\n';
    //        }
    //        return crow::response { os.str() };
    //    });

    crow::logger::setLogLevel(crow::LogLevel::Info);
    app.port(port).multithreaded().run();
}
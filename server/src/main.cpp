
//#include "HttpServer.h"
#include "crow_all.h"
#include <iostream>

// int main()
// {
//     crow::SimpleApp app;

//     CROW_ROUTE(app, "/")([](){
//         return "Hello world";
//     });

//     app.port(18080).multithreaded().run();
// }
// int main(int argc, char * argv[])
//{
//    try
//    {
//        // Check command line arguments.
//        if (argc != 6)
//        {
//            std::cerr << "Usage: http_server_fast <address> <port> <doc_root> <num_workers>\n";
//            std::cerr << "  For IPv4, try:\n";
//            std::cerr << "    http_server_fast 0.0.0.0 80 . 100 block\n";
//            std::cerr << "  For IPv6, try:\n";
//            std::cerr << "    http_server_fast 0::0 80 . 100 block\n";
//            return EXIT_FAILURE;
//        }
//
//        auto const address = boost::asio::ip::make_address(argv[1]);
//        const auto port = static_cast<unsigned short>(std::atoi(argv[2]));
//        std::string doc_root = argv[3];
//        const auto num_workers = std::atoi(argv[4]);
//
//        boost::asio::io_context ioc { 1 };
//        tcp::acceptor acceptor { ioc, { address, port } };
//
//        std::list<HttpWorker> workers;
//        for (int i = 0; i < num_workers; ++i)
//        {
//            workers.emplace_back(acceptor, doc_root);
//            workers.back().start();
//        }
//
//        ioc.run();
//    }
//    catch (const std::exception & e)
//    {
//        std::cerr << "Error: " << e.what() << std::endl;
//        return EXIT_FAILURE;
//    }
//}

class StaticFileServer
{
public:
    StaticFileServer(const std::string & root)
    : m_prefix(root)
    {
    }

    void serveFile(const std::string & url, crow::response & res)
    {
        const auto fullPath = m_prefix + url;

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
        res.code = 404;
        res.end();
    }

private:
    static std::string getMimeType(const std::string & filePath)
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

    std::string m_prefix;
};

int main()
{
    crow::SimpleApp app;

    StaticFileServer sfs("./dist/");

    CROW_ROUTE(app, "/about")
    ([]() { return "About Crow example."; });

    // simple json response
    CROW_ROUTE(app, "/json")
    ([] {
        crow::json::wvalue x;
        x["message"] = "Hello, World!";
        return x;
    });

    CROW_ROUTE(app, "/add/<int>/<int>")
    ([](const crow::request & /*req*/, crow::response & res, int a, int b) {
        std::ostringstream os;
        os << a + b;
        res.write(os.str());
        res.end();
    });

    // Compile error with message "Handler type is mismatched with URL paramters"
    // CROW_ROUTE(app,"/another/<int>")
    //([](int a, int b){
    // return crow::response(500);
    //});

    // more json example
    CROW_ROUTE(app, "/add_json")
    ([](const crow::request & req) {
        auto x = crow::json::load(req.body);
        if (!x)
            return crow::response(400);
        int sum = x["a"].i() + x["b"].i();
        std::ostringstream os;
        os << sum;
        return crow::response { os.str() };
    });

    CROW_ROUTE(app, "/params")
    ([](const crow::request & req) {
        std::ostringstream os;
        os << "Params: " << req.url_params << "\n\n";
        os << "The key 'foo' was " << (req.url_params.get("foo") == nullptr ? "not " : "") << "found.\n";
        if (req.url_params.get("pew") != nullptr)
        {
            double countD = boost::lexical_cast<double>(req.url_params.get("pew"));
            os << "The value of 'pew' is " << countD << '\n';
        }
        auto count = req.url_params.get_list("count");
        os << "The key 'count' contains " << count.size() << " value(s).\n";
        for (const auto & countVal : count)
        {
            os << " - " << countVal << '\n';
        }
        return crow::response { os.str() };
    });

    /* CROW_ROUTE(app, "/")
     ([&sfs](const crow::request & req, crow::response & res) { sfs.serveFile("index.html", res); });*/

    CROW_ROUTE(app, "/<string>")
    ([&sfs](const crow::request & req, crow::response & res, const std::string filename) {
        sfs.serveFile(filename, res);
    });

    CROW_ROUTE(app, "/")
    ([&sfs](const crow::request & req, crow::response & res) {
        int x = 0;
        auto url = req.url;
        if (url == "/")
        {
            sfs.serveFile("index.html", res);
            return;
        }
        sfs.serveFile(url, res);
    });

    crow::logger::setLogLevel(crow::LogLevel::Debug);
    app.port(18080).multithreaded().run();
}

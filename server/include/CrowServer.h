#pragma once

#include <string>

class CrowServer
{
public:
    void run(uint16_t port);
};

namespace crow {
    struct response;
}

class StaticFileServer
{
public:
    StaticFileServer(const std::string & root)
            : m_prefix(root)
    {
    }

    void serveFile(const std::string & url, crow::response & res);

private:

    static std::string getMimeType(const std::string & filePath);

    std::string m_prefix;
};
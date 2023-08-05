#pragma once

#include <rapidjson/document.h>

#include <utility>
#include <vector>

namespace ppp {
class ConfigLoader
{
public:
    explicit ConfigLoader(std::string filePathOrContent);

    bool has(const std::vector<std::string> & nodePath) const;

    const rapidjson::Value & get(const std::vector<std::string> & nodePath) const;

private:
    std::string m_currentDir;
    rapidjson::Document m_config;
};
} // namespace ppp

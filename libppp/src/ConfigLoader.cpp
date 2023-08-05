#include "ConfigLoader.h"
#include "Utilities.h"

#include <fstream>
#include <streambuf>
#include <utility>

namespace ppp {

ConfigLoader::ConfigLoader(std::string filePathOrContent)
{
    if (filePathOrContent.empty()) {
        filePathOrContent = "config.json";
    }
    std::ifstream ifs(filePathOrContent.c_str());
    if (ifs.good()) {
        // We are on a filesystem, let's get the config's directory
        const auto found = filePathOrContent.find_last_of("/\\");
        m_currentDir = found != std::string::npos ? filePathOrContent.substr(0, found) : "";
        const std::string configString((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
        m_config.Parse(configString.c_str());
    } else {
        m_config.Parse(filePathOrContent);
    }
}

bool ConfigLoader::has(const std::vector<std::string> & nodePath) const
{
    const rapidjson::Value * v = &m_config;
    for (const auto & k : nodePath) {
        if (!v->HasMember(k)) {
            return false;
        }
        v = &(*v)[k];
    }
    return true;
}

const rapidjson::Value & ConfigLoader::get(const std::vector<std::string> & nodePath) const
{
    if (nodePath.empty()) {
        return m_config;
    }

    const rapidjson::Value * v = &m_config;
    for (const auto & k : nodePath) {
        if (!v->HasMember(k)) {
            throw std::runtime_error(k + " was not found in object");
        }
        v = &(*v)[k];
    }
    return *v;
}

} // namespace ppp

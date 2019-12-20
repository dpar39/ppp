#pragma once

#include <functional>
#include <istream>
#include <unordered_map>
#include <utility>
#include <vector>

#include <rapidjson/document.h>

struct emscripten_fetch_t;

using PathMapper = std::function<std::string(const std::string &)>;

using ResourceLoadResult = std::function<void(bool, std::istream &)>;

using ResourceLoaded = std::function<void()>;

namespace ppp
{
class ConfigLoader
{
public:
    explicit ConfigLoader(std::string filePathOrContent, ResourceLoaded completeCallback = nullptr);

    rapidjson::Value & get(const std::vector<std::string> & nodePath);

    void loadResource(const std::vector<std::string> & nodePath, const ResourceLoadResult & callback);

private:
    std::string _currentDir;
    rapidjson::Document m_config;
    ResourceLoaded _loadedCallback;

    std::unordered_map<emscripten_fetch_t *, ResourceLoadResult> _callbacks;
};
} // namespace ppp

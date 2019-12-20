
#include "ConfigLoader.h"
#include "Utilities.h"

#ifdef EMSCRIPTEN
#include <emscripten/fetch.h>
#include <iostream>
#else
#include <fstream>
#include <streambuf>
#include <utility>
#endif

namespace ppp
{

struct Membuf : std::streambuf
{
    Membuf(char const * base, const size_t size)
    {
        char * p(const_cast<char *>(base));
        this->setg(p, p, p + size);
    }
    ~Membuf() override = default;
};
struct Imemstream : virtual Membuf, std::istream
{
    Imemstream(char const * base, const size_t size)
    : Membuf(base, size)
    , std::istream(static_cast<std::streambuf *>(this))
    {
    }
};

ConfigLoader::ConfigLoader(std::string filePathOrContent, ResourceLoaded completeCallback)
: _loadedCallback(std::move(completeCallback))
{
    if (filePathOrContent.empty())
    {
        filePathOrContent = "config.json";
    }
    std::ifstream ifs(filePathOrContent, std::ios_base::in);
    if (ifs.good())
    {
        // We are on a filesystem, let's get the config's directory
        const auto found = filePathOrContent.find_last_of("/\\");
        _currentDir = found != std::string::npos ? filePathOrContent.substr(0, found) : "";
        const std::string configString((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
        m_config.Parse(configString.c_str());
    }
    else
    {
        m_config.Parse(filePathOrContent);
    }
}

rapidjson::Value & ConfigLoader::get(const std::vector<std::string> & nodePath)
{
    if (nodePath.empty())
        return m_config;

    rapidjson::Value * v = &m_config;
    for (const auto & k : nodePath)
    {
        if (!v->HasMember(k))
            throw std::runtime_error(k + " was not found in object");
        v = &(*v)[k];
    }
    return *v;
}

void ConfigLoader::loadResource(const std::vector<std::string> & nodePath, const ResourceLoadResult & callback)
{
    const auto & v = get(nodePath);
    if (v.HasMember("data") && v["data"].GetType() == rapidjson::kStringType)
    {
        const auto & item = v["data"];
        const auto fileContent = item.GetString();
        const auto fileContentLen = item.GetStringLength();
        static const std::string AS_BASE64 = "base64 ";

        if (v.HasMember("embed") && v["embed"].GetType() == rapidjson::kStringType
            && AS_BASE64 == v["embed"].GetString())
        {
            auto binaryData = Utilities::base64Decode(fileContent, fileContentLen);
            Imemstream stream(reinterpret_cast<char *>(&binaryData[0]), binaryData.size());
            callback(true, stream);
        }
        else
        {
            Imemstream stream(fileContent, fileContentLen);
            callback(true, stream);
        }
    }

    if (!v.HasMember("file"))
        throw std::runtime_error("No 'file' or 'data' field when trying to load resource");

    const auto resourcePath = v["file"].GetString();
    const auto fullPath = _currentDir.empty() ? resourcePath : _currentDir + "/" + resourcePath;

#ifdef EMSCRIPTEN
    emscripten_fetch_attr_t attr;
    emscripten_fetch_attr_init(&attr);
    strcpy(attr.requestMethod, "GET");
    attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY;
    static std::unordered_map<emscripten_fetch_t *, ConfigLoader *> g_loaders = {};
    attr.onsuccess = [](emscripten_fetch_t * fetch) {
        auto m = Imemstream(fetch->data, fetch->numBytes);
        const auto that = g_loaders.at(fetch);
        auto & callback = that->_callbacks.at(fetch);

        callback(true, m);
        that->_callbacks.erase(fetch);

        emscripten_fetch_close(fetch);
        if (that->_callbacks.empty() && that->_loadedCallback)
        {
            that->_loadedCallback();
        }
    };
    attr.onerror = [](emscripten_fetch_t * fetch) {
        auto m = Imemstream(nullptr, 0);
        const auto that = g_loaders.at(fetch);
        auto & callback = that->_callbacks.at(fetch);

        callback(false, m);
        that->_callbacks.erase(fetch);

        emscripten_fetch_close(fetch);
        if (that->_callbacks.empty() && that->_loadedCallback)
        {
            that->_loadedCallback();
        }
    };
    auto fetch = emscripten_fetch(&attr, fullPath.c_str());
    g_loaders[fetch] = this;
    _callbacks[fetch] = callback;
#else
    // Here things are synchronous
    auto fs = std::ifstream(fullPath.c_str(), std::ios::binary);
    callback(fs.good(), fs);
    if (_loadedCallback)
    {
        _loadedCallback();
    }
#endif
}

} // namespace ppp

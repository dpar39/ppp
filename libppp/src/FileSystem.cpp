
#include "FileSystem.h"
#include "Utilities.h"
#include <rapidjson/document.h>

#ifdef EMSCRIPTEN
#include <emscripten/fetch.h>
#include <iostream>
#include <unordered_map>
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

// void persistFileToIndexedDB(const char * outputFilename, uint8_t * data, size_t numBytes)
//{
//    emscripten_fetch_attr_t attr;
//    emscripten_fetch_attr_init(&attr);
//    strcpy(attr.requestMethod, "EM_IDB_STORE");
//    attr.attributes = EMSCRIPTEN_FETCH_REPLACE | EMSCRIPTEN_FETCH_PERSIST_FILE;
//    attr.requestData = (char *)data;
//    attr.requestDataSize = numBytes;
//    attr.onsuccess = success;
//    attr.onerror = failure;
//    emscripten_fetch(&attr, outputFilename);
//}

PathMapper FileSystem::s_pathMapper = [](const std::string & s) -> std::string { return s; };

void FileSystem::loadFile(const std::string & filePathOrUrl, FileLoadResult callback)
{
    const auto fullPath = s_pathMapper(filePathOrUrl);
#ifdef EMSCRIPTEN
    emscripten_fetch_attr_t attr;
    emscripten_fetch_attr_init(&attr);
    strcpy(attr.requestMethod, "GET");
    attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY | EMSCRIPTEN_FETCH_SYNCHRONOUS;

    static std::unordered_map<emscripten_fetch_t *, FileLoadResult> g_callbacks = {};
    attr.onsuccess = [](emscripten_fetch_t * fetch) {
        auto m = Imemstream(fetch->data, fetch->numBytes);
        g_callbacks.at(fetch)(true, m);
        g_callbacks.erase(fetch);
        emscripten_fetch_close(fetch);

        if (g_callbacks.empty())
        {
        }
    };
    attr.onerror = [](emscripten_fetch_t * fetch) {
        auto m = Imemstream(nullptr, 0);
        g_callbacks.at(fetch)(false, m);
        g_callbacks.erase(fetch);
        emscripten_fetch_close(fetch);
    };
    auto fetch = emscripten_fetch(&attr, fullPath.c_str());
    g_callbacks[fetch] = callback;
    /*auto fetch = emscripten_fetch(&attr, fullPath.c_str());
    if (fetch->status == 200)
    {
        std::cout << ">>> Read " << fetch->numBytes << std::endl;
        auto m = Imemstream(fetch->data, fetch->numBytes);
        callback(true, m);
    }
    else
    {
        auto m = Imemstream(nullptr, 0);
        callback(false, m);
    }*/
#else
    auto fs = std::ifstream(fullPath.c_str(), std::ios::binary);
    callback(fs.good(), fs);
#endif
}

void FileSystem::setPathMapper(const PathMapper pathMapper)
{
    s_pathMapper = std::move(pathMapper);
}

ConfigLoader::ConfigLoader(std::string filePathOrContent, ConfigLoaded completeCallback)
: _loadedCallback(std::move(completeCallback))
{
    if (filePathOrContent.empty())
    {
        filePathOrContent = "config.json";
    }

    std::ifstream ifs(filePathOrContent, std::ios_base::in);
    rapidjson::Document config;
    if (ifs.good())
    {
        // We are on a filesystem, let's get the config's directory
        const auto found = filePathOrContent.find_last_of("/\\");
        _currentDir = found != std::string::npos ? filePathOrContent.substr(0, found) : "";
        const std::string configString((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
        config.Parse(configString.c_str());
    }
    else
    {
        config.Parse(filePathOrContent);
    }
}

// void ConfigLoader::fetchConfigFile(const rapidjson::Value & v, FileLoadResult result)
//{
//    if (v.HasMember("data"))
//    {
//        const auto & item = v["data"];
//        const auto fileContent = item.GetString();
//        const auto fileContentLen = item.GetStringLength();
//
//        static const std::string asBase64 = "base64 ";
//        if (asBase64 == v["embed"].GetString())
//        {
//            auto spData = Utilities::base64Decode(fileContent, fileContentLen);
//            Imemstream stream(reinterpret_cast<char *>(&spData[0]), spData.size());
//            result(true, stream);
//        }
//    }
//
//    if (!v.HasMember("file"))
//        throw std::runtime_error("No 'file' or 'data' field when trying to load resource");
//
//    const auto resourcePath = v["file"].GetString();
//
//    const std::string fullPath = _currentDir.empty() ? _currentDir + "/" + resourcePath : resourcePath;
//#ifdef EMSCRIPTEN
//    emscripten_fetch_attr_t attr;
//    emscripten_fetch_attr_init(&attr);
//    strcpy(attr.requestMethod, "GET");
//    attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY;
//
//    attr.onsuccess = [](emscripten_fetch_t * fetch) {
//        auto m = Imemstream(fetch->data, fetch->numBytes);
//        g_callbacks.at(fetch)(true, m);
//        g_callbacks.erase(fetch);
//        emscripten_fetch_close(fetch);
//
//    };
//    attr.onerror = [](emscripten_fetch_t * fetch) {
//        auto m = Imemstream(nullptr, 0);
//        g_callbacks.at(fetch)(false, m);
//        g_callbacks.erase(fetch);
//        emscripten_fetch_close(fetch);
//    };
//    auto fetch = emscripten_fetch(&attr, fullPath.c_str());
//    g_callbacks[fetch] = callback;
//
//    // auto fetch = emscripten_fetch(&attr, fullPath.c_str());
//    // if (fetch->status == 200)
//    // {
//    //     std::cout << ">>> Read " << fetch->numBytes << std::endl;
//    //     auto m = Imemstream(fetch->data, fetch->numBytes);
//    //     callback(true, m);
//    // }
//    // else
//    // {
//    //     auto m = Imemstream(nullptr, 0);
//    //     callback(false, m);
//    // }
//#else
//    auto fs = std::ifstream(fullPath.c_str());
//    callback(fs.good(), fs);
//    // if (fs.good())
//    //     fileContent.assign(std::istreambuf_iterator<char>(s), std::istreambuf_iterator<char>())
//    callback(fileContent);
//#endif
//}

} // namespace ppp

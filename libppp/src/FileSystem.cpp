
#include "FileSystem.h"

#ifdef EMSCRIPTEN
#include <emscripten/fetch.h>
#include <iostream>
#include <unordered_map>
#else
#include <fstream>
#include <streambuf>
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
    auto fullPath = s_pathMapper(filePathOrUrl);
#ifdef EMSCRIPTEN
    emscripten_fetch_attr_t attr;
    emscripten_fetch_attr_init(&attr);
    strcpy(attr.requestMethod, "GET");
    attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY;

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

    // auto fetch = emscripten_fetch(&attr, fullPath.c_str());
    // if (fetch->status == 200)
    // {
    //     std::cout << ">>> Read " << fetch->numBytes << std::endl;
    //     auto m = Imemstream(fetch->data, fetch->numBytes);
    //     callback(true, m);
    // }
    // else
    // {
    //     auto m = Imemstream(nullptr, 0);
    //     callback(false, m);
    // }
#else
    auto fs = std::ifstream(fullPath.c_str());
    callback(fs.good(), fs);
    // if (fs.good())
    //     fileContent.assign(std::istreambuf_iterator<char>(s), std::istreambuf_iterator<char>()) callback(fileContent);
#endif
}

void FileSystem::setPathMapper(PathMapper pathMapper)
{
    s_pathMapper = pathMapper;
}

} // namespace ppp

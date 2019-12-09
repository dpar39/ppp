
#include "FileSystem.h"

#ifdef EMSCRIPTEN
#include <emscripten/fetch.h>
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

void FileSystem::loadFile(const std::string & filePathOrUrl, FileLoadResult callback)
{
#ifdef EMSCRIPTEN
    emscripten_fetch_attr_t attr;
    emscripten_fetch_attr_init(&attr);
    strcpy(attr.requestMethod, "GET");
    attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY | EMSCRIPTEN_FETCH_PERSIST_FILE;

    static std::unordered_map<emscripten_fetch_t *, FileLoadResult> g_callbacks = {};
    attr.onsuccess = [](emscripten_fetch_t * fetch) {
        emscripten_fetch_close(fetch);
        auto m = Imemstream(fetch->data, fetch->numBytes);
        g_callbacks.at(fetch)(true, m);
        g_callbacks.erase(fetch);
        emscripten_fetch_close(fetch);
    };
    attr.onerror = [](emscripten_fetch_t * fetch) {
        auto m = Imemstream(nullptr, 0);
        g_callbacks.at(fetch)(false, m);
        g_callbacks.erase(fetch);
        emscripten_fetch_close(fetch);
    };
    auto fetch = emscripten_fetch(&attr, filePathOrUrl.c_str());
    g_callbacks[fetch] = callback;
#else
    auto fs = std::ifstream(filePathOrUrl.c_str());
    callback(fs.good(), fs);
#endif
}

} // namespace ppp

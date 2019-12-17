#pragma once

#include "CommonHelpers.h"

#include <functional>
#include <istream>
#include <utility>

struct emscripten_fetch_t;

using PathMapper = std::function<std::string(const std::string &)>;

using FileLoadResult = std::function<void(bool, std::istream &)>;

using FileProcessedCallback = std::function<void()>;

namespace ppp
{
FWD_DECL(FileSystem);

class FileSystem
{
public:
    static void loadFile(const std::string & filePathOrUrl, FileLoadResult callback);

    static void setPathMapper(PathMapper pathMapper);

    static void onFileLoaded(FileProcessedCallback fileProcessedCallback);

private:
    static PathMapper s_pathMapper;

    static FileProcessedCallback s_fileProcessedCallback;
};

using ConfigLoaded = std::function<void(bool)>;

class ConfigLoader
{
public:
    explicit ConfigLoader(std::string filePathOrContent, ConfigLoaded completeCallback = nullptr);

private:
    ConfigLoaded _loadedCallback;

    std::string _currentDir;

    // std::unordered_map<emscripten_fetch_t *, FileLoadResult> g_callbacks = {};
};
} // namespace ppp

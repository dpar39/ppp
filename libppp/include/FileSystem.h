#pragma once

#include "CommonHelpers.h"

#include <functional>
#include <istream>

using PathMapper = std::function<std::string(const std::string &)>;

using FileLoadResult = std::function<void(bool, std::istream &)>;

namespace ppp
{
FWD_DECL(FileSystem);

class FileSystem
{
public:
    static void loadFile(const std::string & filePathOrUrl, FileLoadResult callback);

    static void setPathMapper(PathMapper pathMapper);

private:
    static PathMapper s_pathMapper;
};
} // namespace ppp

#pragma once

#include "CommonHelpers.h"

#include <functional>
#include <istream>

using FileLoadResult = std::function<void(bool, std::istream &)>;

namespace ppp
{
FWD_DECL(FileSystem);

class FileSystem
{
public:
    static void loadFile(const std::string & filePathOrUrl, FileLoadResult callback);

private:
};
} // namespace ppp

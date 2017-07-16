#ifndef NATIVELOG_H
#define NATIVELOG_H

#include <string>

void _log(const char *format, ...) {
    std::string formatStr = format;
    formatStr += "\n";

    va_list args;
    va_start (args, format);
    vprintf(formatStr.c_str(), args);
    va_end(args);
}

#endif //NATIVELOG_H

#include <iostream>

#include "libppp.h"
#include <fstream>

int main(int argc, char ** argv)
{
    PublicPppEngine engine;

    // Read configuration file
    const auto configFilePath = "config.bundle.json";
    std::ifstream fs(configFilePath, std::ios_base::in);
    std::string configString((std::istreambuf_iterator<char>(fs)), std::istreambuf_iterator<char>());

    engine.configure(configString.c_str());

    return 0;
}

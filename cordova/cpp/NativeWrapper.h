#pragma once
#include <memory>

class PublicPppEngine;

class NativeWrapper {
public:
    static void configure(const char *jsonConfig);

    static const char *setImage(const char *imgBase64);

    static const char *detectLandMarks(const char *imgKey); 

    static const char *createTiledPrint(const char *imgKey, const char *request);
private:
    static std::shared_ptr<PublicPppEngine> s_enginePtr;
};
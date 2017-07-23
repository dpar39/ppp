#pragma once
#include <memory>
#include <libppp.h>


class TestCpp {
public:
    TestCpp();
    int testMethod(int p1, const char *p2, double p3, bool p4);
    static const char* staticTestMethod();

private:
    std::shared_ptr<PublicPppEngine> m_enginePtr;
};
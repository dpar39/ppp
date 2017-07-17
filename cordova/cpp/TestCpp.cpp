#include "TestCpp.h"
#include "NativeLog.h"

int TestCpp::testMethod(int p1, const char *p2, double p3, bool p4) {
    _log("testMethod: %d, %s, %lf, %d", p1, p2, p3, p4);
    return p1 + 1;
}

const char* TestCpp::staticTestMethod() {
    return "Olivia Rocks!";
}
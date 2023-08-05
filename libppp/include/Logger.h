#pragma once

#include <chrono>
#include <iostream>
#include <string>

#include <glog/logging.h>

class Timer
{
    std::string m_methodName;
    std::chrono::high_resolution_clock::time_point m_start;

public:
    explicit Timer(std::string methodName)
    : m_methodName(std::move(methodName))
    , m_start(std::chrono::high_resolution_clock::now())
    {
    }
    ~Timer()
    {
        // After function call
        auto stop = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - m_start);
        VLOG(1) << ">> " << m_methodName << ": " << duration.count() / 1000.0 << "ms";
    }
};

#define TIME_METHOD Timer __timer__(__func__);

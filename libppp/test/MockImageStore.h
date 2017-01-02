#pragma once

#include <gmock/gmock.h>
#include "IImageStore.h"

class MockImageStore : public IImageStore
{
public:
    MOCK_METHOD1(setImage, std::string (const cv::Mat&));
    MOCK_METHOD1(getImage, const cv::Mat& (const std::string&));
    MOCK_METHOD1(containsImage, bool (const std::string&));
};

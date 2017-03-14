#pragma once

#include <gmock/gmock.h>
#include "IImageStore.h"

class MockImageStore : public IImageStore
{
public:
    MOCK_METHOD1(setImage, std::string (const cv::Mat&));
    MOCK_METHOD1(getImage, cv::Mat(const std::string&));
    MOCK_METHOD1(unlockImage, void(const std::string&));
    MOCK_METHOD1(containsImage, bool(const std::string&));
    MOCK_METHOD1(setStoreSize, void (size_t));
    
};

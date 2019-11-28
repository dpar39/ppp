#pragma once

#include "IImageStore.h"
#include <gmock/gmock.h>

namespace ppp
{

class MockImageStore : public IImageStore
{
public:
    MOCK_METHOD1(getImage, cv::Mat(const std::string &));
    MOCK_METHOD1(getExifInfo, easyexif::EXIFInfoSPtr(const std::string &));
    MOCK_METHOD1(getLandMarks, LandMarksSPtr(const std::string &));

    MOCK_METHOD1(unlockImage, void(const std::string &));
    MOCK_METHOD1(containsImage, bool(const std::string &));
    MOCK_METHOD1(setStoreSize, void(size_t));

    MOCK_METHOD1(setImage, std::string(const std::string &));
    MOCK_METHOD2(setImage, std::string(const char *, size_t));
};
} // namespace ppp

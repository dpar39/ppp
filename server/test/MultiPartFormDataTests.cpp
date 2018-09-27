//
// Created by Darien Pardinas Diaz on 9/23/18.
//

#include "MultiPartFormParser.h"

#include <gtest/gtest.h>
#include <unordered_map>

class MultiPartFormDataTest : public ::testing::Test
{
};

TEST_F(MultiPartFormDataTest, parse)
{
    const char * body = "------WebKitFormBoundaryq66day34BHUKvTaX\r\nContent-Disposition: form-data; name=\"uploads\"; "
                        "filename=\"image_0001.jpg\"\r\nContent-Type: "
                        "image/jpeg\r\n\r\nJPEGCONTENT------WebKitFormBoundaryq66day34BHUKvTaX--";

    MultiPartFormParser parser;

    std::unordered_multimap<std::string, std::string> headers;
    headers.insert(
        std::make_pair("Content-Type", "multipart/form-data; boundary=----WebKitFormBoundaryq66day34BHUKvTaX"));

    const bool success = parser.parse(headers, body);

    EXPECT_EQ(std::string("image_0001.jpg"), parser.getContentFilename());
    EXPECT_EQ(std::string("uploads"), parser.getContentName());
    EXPECT_TRUE(success);
}

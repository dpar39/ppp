//
// Created by Darien Pardinas Diaz on 9/23/18.
//

#include "MultiPartFormParser.h"
#include "stdafx.h"
#include <gtest/gtest.h>

class MultiPartFormDataTest : public ::testing::Test
{
};

TEST_F(MultiPartFormDataTest, parse)
{
    const char * body = "------WebKitFormBoundaryq66day34BHUKvTaX\r\nContent-Disposition: form-data; name=\"uploads\"; "
                        "filename=\"image_0001.jpg\"\r\n\Content-Type: "
                        "image/jpeg\r\n\r\nJPEGCONTENT------WebKitFormBoundaryq66day34BHUKvTaX--";

    MultiPartFormParser parser;

    crow::request request;
    request.headers.insert(
        std::make_pair("Content-Type", "multipart/form-data; boundary=----WebKitFormBoundaryq66day34BHUKvTaX"));
    request.body = body;

    const bool success = parser.parse(request);

    EXPECT_EQ(std::string("image_0001.jpg"), parser.getContentFilename());
    EXPECT_EQ(std::string("uploads"), parser.getContentName());
    EXPECT_TRUE(success);
}

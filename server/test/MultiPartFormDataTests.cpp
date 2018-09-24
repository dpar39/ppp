//
// Created by Darien Pardinas Diaz on 9/23/18.
//

#include "stdafx.h"
#include <gtest/gtest.h>
#include "MultiPartFormParser.h"

class MultiPartFormDataTest : public ::testing::Test{

};

TEST_F(MultiPartFormDataTest, parse)
{
    const char * body = R"V0G0N(
------WebKitFormBoundaryq66day34BHUKvTaX
Content-Disposition: form-data; name="uploads[]"; filename="image_0001.jpg"
Content-Type: image/jpeg
JPEGCONTENT
------WebKitFormBoundaryq66day34BHUKvTaX--
)V0G0N";

    MultiPartFormParser parser;

    crow::request request;
    request.headers.insert(std::make_pair("Content-Type","multipart/form-data; boundary=----WebKitFormBoundaryq66day34BHUKvTaX"));
    request.body = body;

    parser.parse(request);

    EXPECT_TRUE(false);
}

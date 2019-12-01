#include "PhotoStandard.h"

#include "TestHelpers.h"
#include <gtest/gtest.h>

namespace ppp
{

TEST(PhotoStandardTests, canLoadAllPredefinedPhotoStandardsFromJson)
{
    const auto photoStandardsFile = resolvePath("webapp/src/app/data/photo-standards.json");

    rapidjson::Document d;
    loadJson(photoStandardsFile, d);
    for (rapidjson::SizeType i = 0; i < d.Size(); i++)
    {
        const auto & ps = d[i]["dimensions"];
        const std::string standardId = d[i]["id"].GetString();
        try
        {
            std::cout << standardId << std::endl;
            PhotoStandard::fromJson(ps);
        }
        catch (const std::exception & ex)
        {
            EXPECT_TRUE(false) << "Failed to load passport standard " << standardId
                               << ". Exception message: " << ex.what() << std::endl;
        }
    }
}
} // namespace ppp

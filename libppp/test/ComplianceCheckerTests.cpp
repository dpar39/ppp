#include <gtest/gtest.h>
#include <memory>

#include "ComplianceChecker.h"
#include "ComplianceResult.h"
#include "ImageStore.h"
#include "PhotoStandard.h"
#include "TestHelpers.h"

using namespace testing;

namespace ppp
{
class ComplianceCheckerTests : public Test
{
protected:
    ComplianceCheckerSPtr m_subject; /* SUT */

    ImageStoreSPtr m_imageStore;

public:
    void SetUp() override
    {
        m_imageStore = std::make_shared<ImageStore>();
        m_subject = std::make_shared<ComplianceChecker>();
    }
};

TEST_F(ComplianceCheckerTests, canCreate)
{
    // m_subject->configure();
}

TEST_F(ComplianceCheckerTests, checkInputDpi)
{
    const auto lowResolDpi = 300.0;
    const auto photoStandard = std::make_shared<PhotoStandard>(2, 2, 1.1875, 0, 0, lowResolDpi, "inch");
    const auto & imageFileName = resolvePath("research/sample_test_images/000.jpg");
    const auto imgKey = m_imageStore->setImage(imageFileName);

    const cv::Point2d crownPos(941, 999);
    const cv::Point2d chinPos(927, 1675);

    const auto results
        = m_subject->checkCompliance(imgKey, photoStandard, crownPos, chinPos, { CHECK_INPUT_RESOLUTION });
    const auto result = results.front();

    EXPECT_TRUE(result->getPassed());

    double inputDpi;
    EXPECT_TRUE(result->getParam(CHECK_INPUT_RESOLUTION, inputDpi));

    EXPECT_GT(inputDpi, lowResolDpi);

    // Now make the check fail  by specifying a resolution well above the actual value for this test
    const auto highResolDpi = 600;
    photoStandard->overrideResolution(highResolDpi);
    const auto results2
        = m_subject->checkCompliance(imgKey, photoStandard, crownPos, chinPos, { CHECK_INPUT_RESOLUTION });
    const auto result2 = results2.front();
    EXPECT_FALSE(result2->getPassed());

    EXPECT_LT(inputDpi, highResolDpi);
}

} // namespace ppp

#include "EyeDetector.h"

#include <gtest/gtest.h>
namespace ppp
{
class EyeDetectorTests : public testing::Test
{

protected:
    void SetUp() override
    {
    }

    EyeDetectorSPtr m_pEyeDetector = std::make_shared<EyeDetector>();
};

TEST_F(EyeDetectorTests, FallbackWorks)
{
    // EyeDetector d;
    // d.configure();
}
} // namespace ppp

#include <gtest/gtest.h>
#include <memory>

#include "ComplianceChecker.h"

using namespace testing;

namespace ppp
{
class ComplianceCheckerTests : public Test
{
protected:
    ComplianceCheckerSPtr m_subject; /* SUT */

public:
    void SetUp() override
    {
        m_subject = std::make_shared<ComplianceChecker>();
    }
};

TEST_F(ComplianceCheckerTests, canCreate)
{
    // m_subject->configure();
}
} // namespace ppp

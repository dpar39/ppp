#include <gtest/gtest.h>
#include <memory>

#include "${class_name}.h"

using namespace testing;

namespace ${namespace}
{
class ${class_name}Tests : public Test
{
protected:
    ${class_name}SPtr m_subject; /* SUT */

public:
    void SetUp() override
    {
        m_subject = std::make_shared<${class_name}>();
    }
};

TEST_F(${class_name}Tests, canCreate)
{
    m_subject->myMethod();
}
}
#pragma once

#include "I${class_name}.h"

namespace ${namespace}
{
FWD_DECL(${class_name});

class ${class_name} : public I${class_name}
{
public:
    void myMethod() override;

    ~${class_name}() = default;
};
}
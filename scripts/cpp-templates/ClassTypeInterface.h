#pragma once
#include "CommonHelpers.h"

namespace ${namespace}
{

FWD_DECL(I${class_name})

class I${class_name} : NonCopyable
{
public:
    /*!@brief TODO:  !*/
    virtual void myMethod() = 0;
    virtual ~I${class_name}() = default;
};
}

//
// Created by Darien Pardinas Diaz on 9/25/18.
//

#pragma once

#include <memory>

#define FWD_DECL(classname) \
    class classname; \
    using classname##SPtr = std::shared_ptr<classname>; \

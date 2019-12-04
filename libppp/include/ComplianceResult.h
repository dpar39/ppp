#pragma once

#include "CommonHelpers.h"
#include <rapidjson/document.h>

namespace ppp
{
FWD_DECL(ComplianceResult);

class ComplianceResult
{
private:
    std::string m_checkName;
    bool m_success;
    std::string m_message;

public:
    ComplianceResult(std::string checkName, bool success, std::string message);

    rapidjson::Value toJson(rapidjson::Document::AllocatorType & alloc) const;
};
} // namespace ppp

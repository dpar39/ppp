
#include "ComplianceResult.h"

namespace ppp
{
ComplianceResult::ComplianceResult(std::string checkName, const bool success, std::string message)
: m_checkName(std::move(checkName))
, m_success(success)
, m_message(std::move(message))
{
}

rapidjson::Value ComplianceResult::toJson(rapidjson::Document::AllocatorType & alloc) const
{
    using namespace rapidjson;
    Value obj(kObjectType);
    obj.AddMember(StringRef(COMPLIANCE_RESULT_CHECK_NAME), m_checkName, alloc);
    obj.AddMember(StringRef(COMPLIANCE_RESULT_SUCCESS), m_success, alloc);
    obj.AddMember(StringRef(COMPLIANCE_RESULT_MESSAGE), m_message, alloc);
    return obj;
}
} // namespace ppp

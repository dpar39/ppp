#include "ComplianceResult.h"

namespace ppp {
ComplianceResult::ComplianceResult(std::string checkName, const bool success, std::string message)
: m_propCheckName(std::move(checkName))
, m_propPassed(success)
, m_propMessage(std::move(message))
{
}

rapidjson::Value ComplianceResult::toJson(rapidjson::Document::AllocatorType & alloc) const
{
    using namespace rapidjson;
    Value obj(kObjectType);
    obj.AddMember(StringRef(STR_CheckName), m_propCheckName, alloc);
    obj.AddMember(StringRef(STR_Passed), m_propPassed, alloc);
    obj.AddMember(StringRef(STR_Message), m_propMessage, alloc);

    for (const auto & [k, v] : m_doubleParams) {
        obj.AddMember(StringRef(k), v, alloc);
    }
    for (const auto & [k, v] : m_integralParams) {
        obj.AddMember(StringRef(k), v, alloc);
    }
    for (const auto & [k, v] : m_stringParams) {
        obj.AddMember(StringRef(k), v, alloc);
    }
    for (const auto & [k, v] : m_booleanParams) {
        obj.AddMember(StringRef(k), v, alloc);
    }

    return obj;
}

} // namespace ppp

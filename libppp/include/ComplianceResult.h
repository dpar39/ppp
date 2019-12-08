#pragma once

#include "CommonHelpers.h"
#include <rapidjson/document.h>
#include <unordered_map>

template <class T>
struct dependent_false : std::false_type
{
};

namespace ppp
{
FWD_DECL(ComplianceResult);

class ComplianceResult
{
    DECL_READONLY_PROPERTY(std::string, CheckName)
    DECL_READONLY_PROPERTY(bool, Passed)
    DECL_READONLY_PROPERTY(std::string, Message)

private:
    std::unordered_map<std::string, std::string> m_stringParams;
    std::unordered_map<std::string, bool> m_booleanParams;
    std::unordered_map<std::string, double> m_doubleParams;
    std::unordered_map<std::string, int> m_integralParams;

public:
    ComplianceResult(std::string checkName, bool success, std::string message);

    template <typename T>
    void setParam(const std::string & paramName, T value)
    {
        using U = typename std::remove_cv<typename std::remove_reference<T>::type>::type;
        if constexpr (std::is_same<U, std::string>::value || std::is_same<U, char *>::value)
        {
            m_stringParams[paramName] = value;
        }
        else if constexpr (std::is_same<U, bool>::value)
        {
            m_booleanParams[paramName] = value;
        }
        else if constexpr (std::is_floating_point<U>::value)
        {
            m_doubleParams[paramName] = value;
        }
        else if constexpr (std::is_integral<U>::value)
        {
            m_integralParams[paramName] = value;
        }
        else
        {
            static_assert(dependent_false<T>::value, "type must be double, float, int, string or ");
        }
    }

    template <typename T>
    bool getParam(const std::string & paramName, T & value)
    {
        const auto getVal = [&paramName](const auto & map, auto & v) {
            const auto it = map.find(paramName);
            if (it != map.end())
            {
                v = it->second;
                return true;
            }
            return false;
        };

        if constexpr (std::is_same<T, std::string>::value)
        {
            return getVal(m_stringParams, value);
        }
        else if constexpr (std::is_same<T, bool>::value)
        {
            return getVal(m_booleanParams, value);
        }
        else if constexpr (std::is_floating_point<T>::value)
        {
            return getVal(m_doubleParams, value);
        }
        else if constexpr (std::is_integral<T>::value)
        {
            const auto it = m_integralParams.find(paramName);
            return it->second;
        }
        else
        {
            static_assert(dependent_false<T>::value, "type must be double, float, int, string or boolean");
        }
    }

    rapidjson::Value toJson(rapidjson::Document::AllocatorType & alloc) const;
};
} // namespace ppp

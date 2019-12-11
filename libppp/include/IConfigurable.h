#pragma once
#include <rapidjson/document.h>

class IConfigurable
{
public:
    virtual ~IConfigurable() = default;

    void configure(rapidjson::Value & config)
    {
        m_isConfigured = false;
        configureInternal(config);
    }

    [[nodiscard]] bool isConfigured() const
    {
        return m_isConfigured;
    }

protected:
    virtual void configureInternal(rapidjson::Value & config) = 0;

    bool m_isConfigured = false;
};

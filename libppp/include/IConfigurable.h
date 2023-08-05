#pragma once

#include "CommonHelpers.h"

namespace ppp {
FWD_DECL(ConfigLoader);

class IConfigurable
{
public:
    virtual ~IConfigurable() = default;

    void configure(const ConfigLoaderSPtr & config)
    {
        m_isConfigured = false;
        configureInternal(config);
    }

    [[nodiscard]] bool isConfigured() const
    {
        return m_isConfigured;
    }

protected:
    virtual void configureInternal(const ConfigLoaderSPtr & config) = 0;

    bool m_isConfigured = false;
};
} // namespace ppp

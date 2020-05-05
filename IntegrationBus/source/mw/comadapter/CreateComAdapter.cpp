// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "CreateComAdapter.hpp"

#include "ComAdapter.hpp"

namespace ib {
namespace mw {

auto CreateFastRtpsComAdapterImpl(ib::cfg::Config config, const std::string& participantName) -> std::unique_ptr<IComAdapterInternal>
{
    return std::make_unique<ComAdapter<FastRtpsConnection>>(std::move(config), participantName);
}
auto CreateVAsioComAdapterImpl(ib::cfg::Config config, const std::string& participantName) -> std::unique_ptr<IComAdapterInternal>
{
    return std::make_unique<ComAdapter<VAsioConnection>>(std::move(config), participantName);
}

auto CreateComAdapterImpl(ib::cfg::Config config, const std::string& participantName) -> std::unique_ptr<IComAdapterInternal>
{
    switch (config.middlewareConfig.activeMiddleware)
    {
        // FastRTPS is used as default if unconfigured
    case ib::cfg::Middleware::NotConfigured:
        // [[fallthrough]];
    case ib::cfg::Middleware::FastRTPS:
        return std::make_unique<ComAdapter<FastRtpsConnection>>(std::move(config), participantName);
        
    case ib::cfg::Middleware::VAsio:
        return std::make_unique<ComAdapter<VAsioConnection>>(std::move(config), participantName);

    default:
        throw ib::cfg::Misconfiguration{ "Unknown active middleware selected" };
    }
}

} // mw
} // namespace ib


// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "ib/IbMacros.hpp"

#include <memory>

#include "Config.hpp"

#include "fwd_decl.hpp"
#include "ParentBuilder.hpp"
#include "ReplayBuilder.hpp"

namespace ib {
namespace cfg {
inline namespace deprecated {

class GenericPortBuilder : public ParentBuilder<ParticipantBuilder>
{
public:
    IntegrationBusAPI GenericPortBuilder(ParticipantBuilder *participant, std::string name, mw::EndpointId endpointId);

    IntegrationBusAPI auto WithLink(const std::string& networkName) -> GenericPortBuilder&;
    IntegrationBusAPI auto WithLinkId(int16_t linkId) -> GenericPortBuilder&;
    IntegrationBusAPI auto WithEndpointId(mw::EndpointId id) -> GenericPortBuilder&;

    IntegrationBusAPI auto WithProtocolType(GenericPort::ProtocolType protocolType) -> GenericPortBuilder&;
    IntegrationBusAPI auto WithDefinitionUri(std::string uri) -> GenericPortBuilder&;

    IntegrationBusAPI auto WithTraceSink(std::string sinkName) -> GenericPortBuilder&;
    IntegrationBusAPI auto WithReplay(std::string useTraceSource) -> ReplayBuilder&;

    IntegrationBusAPI auto operator->() -> ParticipantBuilder*;

    IntegrationBusAPI auto Build() -> GenericPort;

private:
    GenericPort _port;
    std::string _link;
    std::unique_ptr<ReplayBuilder> _replayBuilder;
};

// ================================================================================
//  Inline Implementations
// ================================================================================

} // namespace deprecated
} // namespace cfg
} // namespace ib

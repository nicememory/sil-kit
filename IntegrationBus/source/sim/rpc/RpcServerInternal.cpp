// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "RpcServerInternal.hpp"
#include "RpcDatatypeUtils.hpp"
#include "ib/sim/rpc/string_utils.hpp"

namespace ib {
namespace sim {
namespace rpc {

RpcServerInternal::RpcServerInternal(mw::IComAdapterInternal* comAdapter, cfg::RpcPort config, mw::sync::ITimeProvider* timeProvider, CallProcessor handler, IRpcServer* parent)
    : _comAdapter{ comAdapter }, _timeProvider{ timeProvider }, _handler{ std::move(handler) }, _parent{ parent }
{
    _config = std::move(config);
}

void RpcServerInternal::ReceiveIbMessage(const mw::IIbServiceEndpoint* from, const FunctionCall &msg)
{
    //if (from->GetServiceId().legacyEpa == _serviceId.legacyEpa)
    //    return;

    ReceiveMessage(msg);
}

void RpcServerInternal::ReceiveMessage(const FunctionCall& msg) 
{
    if (_handler)
    {
        // NB: We keep the ownership to keep the passed IRpcCallHandle* alive
        auto callHandle = std::make_unique<CallHandleImpl>(msg.callUUID);
        auto callHandlePtr = callHandle.get();
        _receivedCallHandles[to_string(msg.callUUID)] = std::move(callHandle);
        _handler(_parent, callHandlePtr, msg.data);
    }
}

void RpcServerInternal::SubmitResult(IRpcCallHandle* callHandlePtr, const std::vector<uint8_t>& resultData)
{
    auto callHandle = static_cast<const CallHandleImpl&>(*callHandlePtr);
    auto callHandleStr = to_string(callHandle._callUUID);
    auto it = _receivedCallHandles.find(callHandleStr);
    if (it != _receivedCallHandles.end())
    {
        _comAdapter->SendIbMessage(this, FunctionCallResponse{ callHandle._callUUID, resultData });
        _receivedCallHandles.erase(callHandleStr);
    }
}

void RpcServerInternal::SetRpcHandler(CallProcessor handler)
{
    _handler = std::move(handler);
}

void RpcServerInternal::SetTimeProvider(mw::sync::ITimeProvider* provider)
{
    _timeProvider = provider;
}

} // namespace rpc
} // namespace sim
} // namespace ib

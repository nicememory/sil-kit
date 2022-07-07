// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "MessageBuffer.hpp"

#include "silkit/services/can/CanDatatypes.hpp"

namespace SilKit {
namespace Services {
namespace Can {

void Serialize(SilKit::Core::MessageBuffer& buffer,const Services::Can::CanFrameEvent& msg);
void Serialize(SilKit::Core::MessageBuffer& buffer,const Services::Can::CanFrameTransmitEvent& msg);
void Serialize(SilKit::Core::MessageBuffer& buffer,const Services::Can::CanControllerStatus& msg);
void Serialize(SilKit::Core::MessageBuffer& buffer,const Services::Can::CanConfigureBaudrate& msg);
void Serialize(SilKit::Core::MessageBuffer& buffer,const Services::Can::CanSetControllerMode& msg);

void Deserialize(SilKit::Core::MessageBuffer& buffer, Services::Can::CanFrameEvent& out);
void Deserialize(SilKit::Core::MessageBuffer& buffer, Services::Can::CanFrameTransmitEvent& out);
void Deserialize(SilKit::Core::MessageBuffer& buffer, Services::Can::CanControllerStatus& out);
void Deserialize(SilKit::Core::MessageBuffer& buffer, Services::Can::CanConfigureBaudrate& out);
void Deserialize(SilKit::Core::MessageBuffer& buffer, Services::Can::CanSetControllerMode& out);

} // namespace Can    
} // namespace Services
} // namespace SilKit
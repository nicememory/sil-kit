/* Copyright (c) 2022 Vector Informatik GmbH

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#include <chrono>
#include <functional>
#include <string>

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "silkit/services/can/string_utils.hpp"
#include "silkit/util/HandlerId.hpp"

#include "MockParticipant.hpp"
#include "MockTraceSink.hpp"

#include "CanController.hpp"
#include "CanDatatypesUtils.hpp"

namespace {

using namespace std::chrono_literals;

using testing::Return;
using testing::A;
using testing::An;
using testing::_;
using testing::InSequence;
using testing::NiceMock;

using namespace SilKit::Core;
using namespace SilKit::Services::Can;
using SilKit::Util::HandlerId;

using SilKit::Core::Tests::DummyParticipant;

MATCHER_P(CanTransmitAckWithouthTransmitIdMatcher, truthAck, "") {
    *result_listener << "matches CanTransmitAcks without checking the transmit id";
    auto frame1 = truthAck;
    auto frame2 = arg;
    return frame1.canId == frame2.canId && frame1.status == frame2.status && frame1.timestamp == frame2.timestamp;
}

class MockParticipant : public DummyParticipant
{
public:
    MOCK_METHOD2(SendMsg, void(const IServiceEndpoint*, const WireCanFrameEvent&));
    MOCK_METHOD2(SendMsg, void(const IServiceEndpoint*, const CanFrameTransmitEvent&));
    MOCK_METHOD2(SendMsg, void(const IServiceEndpoint*, const CanConfigureBaudrate&));
    MOCK_METHOD2(SendMsg, void(const IServiceEndpoint*, const CanSetControllerMode&));
};

class CanControllerCallbacks
{
public:
    MOCK_METHOD2(FrameHandler, void(ICanController*, CanFrameEvent));
    MOCK_METHOD2(StateChangeHandler, void(ICanController*, CanStateChangeEvent));
    MOCK_METHOD2(ErrorStateChangeHandler, void(ICanController*, CanErrorStateChangeEvent));
    MOCK_METHOD2(FrameTransmitHandler, void(ICanController*, CanFrameTransmitEvent));
};

class CanControllerTrivialSimTest : public testing::Test
{
public:
    CanControllerTrivialSimTest() {};
};

TEST(CanControllerTrivialSimTest, send_can_frame)
{
    MockParticipant mockParticipant;

    ServiceDescriptor senderDescriptor{};
    senderDescriptor.SetParticipantName("canControllerPlaceholder");
    senderDescriptor.SetServiceId(17);
    SilKit::Config::CanController cfg;

    CanController canController(&mockParticipant, cfg, mockParticipant.GetTimeProvider());
    canController.SetServiceDescriptor(senderDescriptor);
    canController.Start();

    WireCanFrameEvent testFrameEvent{};
    testFrameEvent.timestamp = 0ns;
    testFrameEvent.userContext = 0;

    EXPECT_CALL(mockParticipant, SendMsg(&canController, testFrameEvent))
        .Times(1);
    EXPECT_CALL(mockParticipant.mockTimeProvider, Now())
        .Times(1);

    canController.SendFrame(ToCanFrame(testFrameEvent.frame));
}

TEST(CanControllerTrivialSimTest, receive_can_message)
{
    using namespace std::placeholders;

    ServiceDescriptor senderDescriptor{};
    senderDescriptor.SetParticipantName("canControllerPlaceholder");
    senderDescriptor.SetServiceId(17);

    MockParticipant mockParticipant;
    CanControllerCallbacks callbackProvider;
    SilKit::Config::CanController cfg;

    CanController canController(&mockParticipant, cfg, mockParticipant.GetTimeProvider());
    canController.AddFrameHandler(std::bind(&CanControllerCallbacks::FrameHandler, &callbackProvider, _1, _2));
    canController.Start();

    WireCanFrameEvent testFrameEvent{};
    testFrameEvent.frame.canId = 16;
    testFrameEvent.direction = SilKit::Services::TransmitDirection::RX;
    testFrameEvent.userContext = (void*)1234;

    EXPECT_CALL(callbackProvider, FrameHandler(&canController, ToCanFrameEvent(testFrameEvent)))
        .Times(1);

    CanController canControllerPlaceholder(&mockParticipant, cfg, mockParticipant.GetTimeProvider());
    canControllerPlaceholder.SetServiceDescriptor(senderDescriptor);
    canController.ReceiveMsg(&canControllerPlaceholder, testFrameEvent);
}

TEST(CanControllerTrivialSimTest, receive_can_message_rx_filter1)
{
    using namespace std::placeholders;

    ServiceDescriptor senderDescriptor{};
    senderDescriptor.SetParticipantName("canControllerPlaceholder");
    senderDescriptor.SetServiceId(17);

    MockParticipant mockParticipant;
    CanControllerCallbacks callbackProvider;
    SilKit::Config::CanController cfg;
    CanController canController(&mockParticipant, cfg, mockParticipant.GetTimeProvider());
    canController.AddFrameHandler(std::bind(&CanControllerCallbacks::FrameHandler, &callbackProvider, _1, _2),
                                  (SilKit::Services::DirectionMask)SilKit::Services::TransmitDirection::RX);
    canController.Start();

    WireCanFrameEvent testFrameEvent{};
    testFrameEvent.frame.canId = 16;
    testFrameEvent.direction = SilKit::Services::TransmitDirection::RX;
    testFrameEvent.userContext = (void*)1234;

    EXPECT_CALL(callbackProvider, FrameHandler(&canController, ToCanFrameEvent(testFrameEvent))).Times(1);

    CanController canControllerPlaceholder(&mockParticipant, cfg, mockParticipant.GetTimeProvider());
    canControllerPlaceholder.SetServiceDescriptor(senderDescriptor);
    canController.ReceiveMsg(&canControllerPlaceholder, testFrameEvent);
}


TEST(CanControllerTrivialSimTest, receive_can_message_rx_filter2)
{
    using namespace std::placeholders;

    ServiceDescriptor senderDescriptor{};
    senderDescriptor.SetParticipantName("canControllerPlaceholder");
    senderDescriptor.SetServiceId(17);

    MockParticipant mockParticipant;
    CanControllerCallbacks callbackProvider;

    SilKit::Config::CanController cfg;
    CanController canController(&mockParticipant, cfg, mockParticipant.GetTimeProvider());
    canController.AddFrameHandler(std::bind(&CanControllerCallbacks::FrameHandler, &callbackProvider, _1, _2),
                                  (SilKit::Services::DirectionMask)SilKit::Services::TransmitDirection::TX);
    canController.Start();

    WireCanFrameEvent testFrameEvent{};
    testFrameEvent.frame.canId = 16;
    testFrameEvent.direction = SilKit::Services::TransmitDirection::RX;

    EXPECT_CALL(callbackProvider, FrameHandler(&canController, ToCanFrameEvent(testFrameEvent)))
        .Times(0);

    CanController canControllerPlaceholder(&mockParticipant, cfg, mockParticipant.GetTimeProvider());
    canControllerPlaceholder.SetServiceDescriptor(senderDescriptor);
    canController.ReceiveMsg(&canControllerPlaceholder, testFrameEvent);
}

TEST(CanControllerTrivialSimTest, receive_can_message_tx_filter1)
{
    using namespace std::placeholders;

    EndpointAddress senderAddress{ 17, 4 };

    MockParticipant mockParticipant;
    CanControllerCallbacks callbackProvider;

    SilKit::Config::CanController cfg;
    CanController canController(&mockParticipant, cfg, mockParticipant.GetTimeProvider());
    canController.AddFrameHandler(std::bind(&CanControllerCallbacks::FrameHandler, &callbackProvider, _1, _2),
                                  (SilKit::Services::DirectionMask)SilKit::Services::TransmitDirection::TX);
    canController.Start();

    WireCanFrameEvent testFrameEvent{};
    testFrameEvent.frame.canId = 16;
    testFrameEvent.direction = SilKit::Services::TransmitDirection::TX;

    EXPECT_CALL(callbackProvider, FrameHandler(&canController, ToCanFrameEvent(testFrameEvent)))
        .Times(1);
    EXPECT_CALL(mockParticipant.mockTimeProvider, Now()).Times(1);

    CanController canControllerPlaceholder(&mockParticipant, cfg, mockParticipant.GetTimeProvider());
    canControllerPlaceholder.SetServiceDescriptor(from_endpointAddress(senderAddress));
    canController.SendFrame(ToCanFrame(testFrameEvent.frame));
}


TEST(CanControllerTrivialSimTest, receive_can_message_tx_filter2)
{
    using namespace std::placeholders;

    EndpointAddress senderAddress{ 17, 4 };

    MockParticipant mockParticipant;
    CanControllerCallbacks callbackProvider;

    SilKit::Config::CanController cfg;
    CanController canController(&mockParticipant, cfg, mockParticipant.GetTimeProvider());
    canController.AddFrameHandler(std::bind(&CanControllerCallbacks::FrameHandler, &callbackProvider, _1, _2), (SilKit::Services::DirectionMask)SilKit::Services::TransmitDirection::RX);
    canController.Start();

    CanFrameEvent testFrameEvent{};
    testFrameEvent.frame.canId = 16;
    testFrameEvent.direction = SilKit::Services::TransmitDirection::TX;

    EXPECT_CALL(callbackProvider, FrameHandler(&canController, testFrameEvent))
        .Times(0);
    EXPECT_CALL(mockParticipant.mockTimeProvider, Now()).Times(1);

    CanController canControllerPlaceholder(&mockParticipant, cfg, mockParticipant.GetTimeProvider());
    canControllerPlaceholder.SetServiceDescriptor(from_endpointAddress(senderAddress));
    canController.SendFrame(testFrameEvent.frame);
}


/*! \brief Ensure that start, stop, sleep, and reset have no effect
 *
 *  Rationale:
 *   ControllerModes are not supported by the CanController. This feature requires
 *   the usage of a proper CanBusSimulator and CanControllerProxies.
 */
TEST(CanControllerTrivialSimTest, start_stop_sleep_reset)
{
    MockParticipant mockParticipant;
    SilKit::Config::CanController cfg;

    CanController canController(&mockParticipant, cfg, mockParticipant.GetTimeProvider());

    EXPECT_CALL(mockParticipant, SendMsg(A<const IServiceEndpoint*>(), A<const CanSetControllerMode&>()))
        .Times(0);

    canController.Start();
    canController.Stop();
    canController.Sleep();
    canController.Reset();
}

/*! \brief Ensure that SetBaudRate has no effect
*
*  Rationale:
*   BaudRate is not supported by the CanController. This feature requires
*   the usage of a proper CanBusSimulator and CanControllerProxies.
*/
TEST(CanControllerTrivialSimTest, set_baudrate)
{
    MockParticipant mockParticipant;
    SilKit::Config::CanController cfg;

    CanController canController(&mockParticipant, cfg, mockParticipant.GetTimeProvider());

    EXPECT_CALL(mockParticipant, SendMsg(An<const IServiceEndpoint*>(), A<const CanConfigureBaudrate&>()))
        .Times(0);

    canController.SetBaudRate(3000, 500000, 1000000);
}

TEST(CanControllerTrivialSimTest, receive_ack)
{
    using namespace std::placeholders;

    EndpointAddress controllerAddress = { 3, 8 };

    MockParticipant mockParticipant;
    CanControllerCallbacks callbackProvider;
    SilKit::Config::CanController cfg;

    CanController canController(&mockParticipant, cfg, mockParticipant.GetTimeProvider());
    canController.SetServiceDescriptor(from_endpointAddress(controllerAddress));
    canController.AddFrameTransmitHandler(std::bind(&CanControllerCallbacks::FrameTransmitHandler, &callbackProvider, _1, _2));
    canController.Start();

    CanFrame msg{};
    CanFrameTransmitEvent expectedAck{ msg.canId, 0ns, CanTransmitStatus::Transmitted, nullptr };

    EXPECT_CALL(callbackProvider, FrameTransmitHandler(&canController, CanTransmitAckWithouthTransmitIdMatcher(expectedAck)))
        .Times(2);
    canController.SendFrame(msg);
    canController.SendFrame(msg);
}

TEST(CanControllerTrivialSimTest, add_remove_handler)
{
    using namespace std::placeholders;

    MockParticipant mockParticipant;
    CanControllerCallbacks callbackProvider;

    CanController canController(&mockParticipant, {}, mockParticipant.GetTimeProvider());
    canController.Start();

    const int numHandlers = 10;
    std::vector<HandlerId> handlerIds;
    for (int i = 0; i < numHandlers; i++)
    {
        handlerIds.push_back(canController.AddFrameTransmitHandler(
            std::bind(&CanControllerCallbacks::FrameTransmitHandler, &callbackProvider, _1, _2)));
    }
    
    CanFrame msg{};
    CanFrameTransmitEvent expectedAck{msg.canId, 0ns, CanTransmitStatus::Transmitted, nullptr};

    EXPECT_CALL(callbackProvider, FrameTransmitHandler(&canController, CanTransmitAckWithouthTransmitIdMatcher(expectedAck)))
        .Times(numHandlers);

    canController.SendFrame(msg);
    for (auto&& handlerId : handlerIds)
    {
        canController.RemoveFrameTransmitHandler(handlerId);
    }
    
    // All handlers removed, no further calls should appear.
    canController.SendFrame(msg);
}

TEST(CanControllerTrivialSimTest, DISABLED_cancontroller_uses_tracing)
{


    SilKit::Tests::MockTraceSink traceSink;
    Tests::DummyParticipant participant;
    SilKit::Config::CanController cfg;
    const std::chrono::nanoseconds now = 1337ns;
    const EndpointAddress controllerAddress = {1,2};
    const EndpointAddress otherAddress = {2,2};

    ON_CALL(participant.mockTimeProvider, Now())
        .WillByDefault(testing::Return(now));


    CanController controller(&participant, cfg, participant.GetTimeProvider());
    controller.SetServiceDescriptor(from_endpointAddress(controllerAddress));
    controller.AddSink(&traceSink);
    controller.Start();


    WireCanFrameEvent testFrameEvent{};

    //Send direction
    EXPECT_CALL(participant.mockTimeProvider, Now())
        .Times(1);
    EXPECT_CALL(traceSink,
        Trace(SilKit::Services::TransmitDirection::TX, controllerAddress, now, ToCanFrame(testFrameEvent.frame)))
        .Times(1);
    controller.SendFrame(ToCanFrame(testFrameEvent.frame));

    // Receive direction
    EXPECT_CALL(participant.mockTimeProvider, Now())
        .Times(1);
    EXPECT_CALL(traceSink,
        Trace(SilKit::Services::TransmitDirection::RX, controllerAddress, now, ToCanFrame(testFrameEvent.frame)))
        .Times(1);

    CanController canControllerPlaceholder(&participant, {}, participant.GetTimeProvider());
    canControllerPlaceholder.SetServiceDescriptor(from_endpointAddress(otherAddress));
    controller.ReceiveMsg(&canControllerPlaceholder, testFrameEvent);
}

TEST(CanControllerTrivialSimTest, fail_can_frame_not_started)
{
    MockParticipant mockParticipant;

    ServiceDescriptor senderDescriptor{};
    senderDescriptor.SetParticipantName("canControllerPlaceholder");
    senderDescriptor.SetServiceId(17);
    SilKit::Config::CanController cfg;

    CanController canController(&mockParticipant, cfg, mockParticipant.GetTimeProvider());
    canController.SetServiceDescriptor(senderDescriptor);

    WireCanFrameEvent testFrameEvent{};
    testFrameEvent.timestamp = 0ns;
    testFrameEvent.userContext = 0;

    EXPECT_CALL(mockParticipant, SendMsg(&canController, testFrameEvent)).Times(0);
    EXPECT_CALL(mockParticipant.mockTimeProvider, Now()).Times(0);

    // controller is not started when sending can frame
    canController.SendFrame(ToCanFrame(testFrameEvent.frame));
}


}  // anonymous namespace
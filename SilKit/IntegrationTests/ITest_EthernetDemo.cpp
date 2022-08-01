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
#include <memory>
#include <thread>
#include <string>
#include <chrono>
#include <iostream>

#include <unordered_map> //remove this after rebase on cmake-cleanup-branch


#include "silkit/services/ethernet/all.hpp"

#include "gtest/gtest.h"

#include "ITestFixture.hpp"
#include "EthDatatypeUtils.hpp"

namespace {
using namespace SilKit::Tests;
using namespace SilKit::Config;
using namespace SilKit::Services;


TEST_F(ITest_SimTestHarness, ethernet_demo)
{
    //Create a simulation setup with 2 participants 
    SetupFromParticipantList({"EthernetReader", "EthernetWriter"});

    const SilKit::Services::Ethernet::EthernetMac writerMac{00, 22, 33, 44, 55, 66};
    const SilKit::Services::Ethernet::EthernetMac readerMac{00, 88, 99, 00, 11, 22};
    const SilKit::Services::Ethernet::EthernetEtherType etherType{ 0x0800 };

    //Test Data
    auto wireFrame = SilKit::Services::Ethernet::CreateEthernetFrame(readerMac, writerMac, etherType,
      "Hello World! We need a minimum size of 64 bytes for the frame so here is some useless data");
    auto frame = ToEthernetFrame(wireFrame);

    //Test Results
    auto receivedLinkDown = false;
    auto receivedLinkUp = false;
    auto receivedMessage = false;
    auto receivedAckTransmitted = false;
    auto receivedAckDropped = false;
    auto receiveCount = 0;
    auto sendCount = 0;
    auto linkBitrate = 0;
    //Set up the Sending and receiving participants
    {
        /////////////////////////////////////////////////////////////////////////
        // EthernetWriter
        /////////////////////////////////////////////////////////////////////////
        auto&& simParticipant = _simTestHarness->GetParticipant("EthernetWriter");
        auto&& participant = simParticipant->Participant();
        auto* lifecycleService = simParticipant->GetOrCreateLifecycleService();
        auto* timeSyncService = simParticipant->GetOrCreateTimeSyncService();
        auto&& ethernetController = participant->CreateEthernetController("EthernetController1", "ETH1");
        
        lifecycleService->SetCommunicationReadyHandler([ethernetController]() {
            Log() << "---   EthernetWriter: Init called, setting baud rate and starting";
            ethernetController->Activate();
        });

        ethernetController->AddBitrateChangeHandler([&](auto, Ethernet::EthernetBitrateChangeEvent bitrateChangeEvent) {
          linkBitrate = bitrateChangeEvent.bitrate;
        });

        ethernetController->AddStateChangeHandler([&](auto, Ethernet::EthernetStateChangeEvent stateChangeEvent) {
          if (stateChangeEvent.state == Ethernet::EthernetState::LinkDown)
          {
            receivedLinkDown = true;
          }
          if (stateChangeEvent.state == Ethernet::EthernetState::LinkUp)
          {
            receivedLinkUp = true;
          }
        });

        ethernetController->AddFrameTransmitHandler([&](auto, auto ack) {
          if (ack.status == Ethernet::EthernetTransmitStatus::Transmitted)
          {
            receivedAckTransmitted = true;
          }
          if (ack.status == Ethernet::EthernetTransmitStatus::Dropped)
          {
            receivedAckDropped = true;
          }
        });


        timeSyncService->SetSimulationStepHandler(
          [ethernetController, &sendCount, &frame](auto now) {
            // Send while link is down
            if (now == 10ms)
            {
              Log() << "---   EthernetWriter sending EthernetFrame which should cause LinkDown status";
              ethernetController->SendFrame(frame);
            }

            //give the Ethernet link some time to get into the 'UP' state

            //Cause a queue overflow by sending too fast
            if (now == 51ms)
            {
              for (auto i = 0; i < 33;i++) // keep this in sync with EthernetController mTxQueueLimit
              {
                ethernetController->SendFrame(frame);
              }
            }
            // Send controlled number of messages
            if (now > 55ms && (sendCount++ <= 10)) 
            {
              Log() << "---   EthernetWriter sending EthernetFrame";
              ethernetController->SendFrame(frame);
            }
            //Throttle this thread, so it does not starve other participants on the CI
            std::this_thread::sleep_for(10ms);
        }, 1ms);
    }
    auto readerTime = 0ms;
    {
        /////////////////////////////////////////////////////////////////////////
        // EthernetReader
        /////////////////////////////////////////////////////////////////////////
        auto&& simParticipant = _simTestHarness->GetParticipant("EthernetReader");
        auto&& participant = simParticipant->Participant();
        auto* lifecycleService = simParticipant->GetOrCreateLifecycleService();
        auto* timeSyncService = simParticipant->GetOrCreateTimeSyncService();
        auto&& ethernetController = participant->CreateEthernetController("EthernetController2", "ETH1");

        timeSyncService->SetSimulationStepHandler([&](auto now) {
          readerTime = std::chrono::duration_cast<std::chrono::milliseconds>(now);
        }, 1ms);

        lifecycleService->SetCommunicationReadyHandler([ethernetController]() {
            Log() << "---   EthernetReader: Init called, setting baud rate and starting";
            ethernetController->Activate();
        });

        ethernetController->AddFrameHandler(
            [&readerTime, &receivedMessage, &frame, &receiveCount, lifecycleService](auto, const auto& netsimMessage) {
            if (readerTime < 55ms)
            {
              // ignore the messages from the Queue-overflow attempt and LinkUp
              return;
            }

            ASSERT_EQ(frame, netsimMessage.frame);
            if (receiveCount++ == 10)
            {
                receivedMessage = true;
                lifecycleService->Stop("Test done");
                Log() << "---    EthernetReader: Sending Stop ";
            }
        });
    }


    auto ok = _simTestHarness->Run(5s);
    ASSERT_TRUE(ok) << "SimTestHarness should terminate without timeout";

    EXPECT_TRUE(receivedLinkUp) << " Trivial simulation should receive the LinkUp on Activate()";
    EXPECT_FALSE(receivedLinkDown) << "Trivial simulation should not receive LinkDown";
    EXPECT_TRUE(receivedMessage) << "Acknowledged Send must result in a receive";
    EXPECT_TRUE(receivedAckTransmitted) << "Sending  a message must produce an acknowledge";
    EXPECT_FALSE(receivedAckDropped) << "Sending too fast has no effect on trivial simulation ";
    EXPECT_EQ(linkBitrate, 0) << "Bit rate change handler is not called as part of trivial simulation";
}
} //end namespace

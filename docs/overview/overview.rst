﻿.. include:: substitutions.rst

=======================
Vector SIL Kit Overview
=======================

.. |_| unicode:: 0xA0 
   :trim:
.. |CANBuses| replace:: CAN / CAN |_| FD / CAN |_| XL
.. |CAN-API| replace:: :ref:`CAN / CAN FD / CAN XL<chap:can-service-api>`
.. |Ethernet-API| replace:: :ref:`Ethernet<chap:ethernet-service-api>`
.. |Flexray-API| replace:: :ref:`FlexRay<chap:flexray-service-api>`
.. |LIN-API| replace:: :ref:`LIN<chap:lin-service-api>`
.. |Lifecycle-API| replace:: :ref:`lifecycle<chap:lifecycle-service-api>`
.. |Timesync-API| replace:: :ref:`time synchronization<chap:timesync-service-api>`
.. |Logging-API| replace:: :ref:`logging<chap:logging-service-api>`

The Vector |ProductName| is an open source library for connecting Software-in-the-Loop Environments.
It provides:

* Communication infrastructure for automotive and non-automotive applications

* Interoperability between any SIL Kit enabled applications

* Cross platform communication

* Implemented in C++ w/o platform dependencies

* Interoperability between Windows, Linux, and other UNIX derivates​

* Stable API and ABI as well as long-term network layer compatibility

.. _sec:overview-supported-services:

Supported Services
------------------

The |ProductName| supports the following services:

* **Vehicle Network Controllers:** |CAN-API|, |Ethernet-API|, |Flexray-API|, and |LIN-API|
* **Data Publish/Subscribe:** topic-based :ref:`publish/subscribe<chap:pubsub-service-api>` mechanism to exchange arbitrary data
* **RPC Servers/Clients:** for dynamic :ref:`remote procedure execution<chap:rpc-service-api>`
* **Orchestration:** for |Lifecycle-API| and |Timesync-API| control
* **Logging:** for |Logging-API| messages into a file, standard output, or to other participants
* **Custom Network Simulator:** takes control over the message flow to simulate network details

Vehicle Networks
----------------

The |ProductName| provides means to simulate |CANBuses|, Ethernet, FlexRay, and LIN networks.
All networks except for FlexRay can be simulated with two different levels of detail: a simple, functional simulation or a detailed simulation.
The simple simulation assumes no delay and unlimited bandwidth.
The detailed simulation is enabled by using an additional |ProductName| |NetSim|, which considers these aspects as well.
The |NetSim| is not part of |ProductName| itself.
Because of its sensitivity regarding time, FlexRay is only supported in a detailed simulation.

Vehicle networks are accessed through their corresponding services (e.g., a CAN controller).
The service interfaces are the same for simple and detailed simulation.
As a result, an application that works in the simple use case also works when switching to a detailed simulation using the |NetSim|.
The section :ref:`sec:api-services` describes how to configure and use vehicle network services in detail.

Simulation Basics
-----------------

Every simulation setup comprises several components.
The :ref:`SIL Kit Registry<sec:util-registry>` establishes connections between participants in a simulation.
The |ProductName| Registry process is required and must be started as a first step.
It acts as a connection broker between the actual simulation participants.

One or more :doc:`participants<../api/participant>` take care of the actual simulation logic and communicate via built-in :ref:`services<sec:overview-supported-services>`.
|ProductName| supports common vehicle networks (|CANBuses|, Ethernet, FlexRay, LIN), data messages for application specific communication in a publish / subscribe pattern, and Remote Procedure Calls (RPC).
The participant and its services are created by the client application using the |ProductName| :doc:`API<../api/api>`.

To manage a group of participants in a simulation, |ProductName| participants can coordinate their startup behavior.
For example, participants may wait before starting until a given set of required participants have joined the simulation.
Furthermore, |ProductName| participants can use a virtual simulation time that is synchronized with other participants.

While participants are first configured in the |ProductName| API by the developer, they can be reconfigured by the user by providing a YAML-based participant configuration file when starting the participant.
A similar configuration file is available for the |ProductName| Registry as well.

* For getting started operating/orchestrating a |ProductName| simulation, see the :doc:`User Guide <../for-users/users>`.
* For getting started developing with the |ProductName|, see the :doc:`Developer Guide <../for-developers/developers>`.

.. _sec:overview-ecosystem:

The |ProductName| Ecosystem
---------------------------

Related Projects
~~~~~~~~~~~~~~~~

One of the design goals of the |ProductName| is to easily connect different third-party tools,
such as emulators, virtual machines and simulation tools.

The |ProductName| ecosystem comprises the following turn-key solutions:

* The `SIL Kit Adapter for QEMU <https://github.com/vectorgrp/sil-kit-adapters-qemu>`_
  integrates with QEMU to support co-simulation with emulated targets.

* The `SIL Kit Adapter for TAP devices <https://github.com/vectorgrp/sil-kit-adapters-tap>`_
  project provides first-class support for TAP devices of the host operating system.

* The `SIL Kit Adapter for virtual CAN <https://github.com/vectorgrp/sil-kit-adapters-vcan>`_
  can be used to attach a virtual CAN (Controller Area Network) interface (SocketCAN) to a Vector SIL Kit CAN bus.

* The `SIL Kit Adapter for Generic Linux IO <https://github.com/vectorgrp/sil-kit-adapters-generic-linux-io>`_
  allows to attach SIL Kit to a generic Linux IO device, such as character devices or GPIOs.

* The `SIL Kit FMU Importer <https://github.com/vectorgrp/sil-kit-fmu-importer>`_
  allows to import Functional Mockup Units (FMUs) as SIL Kit participants.

Related Applications
~~~~~~~~~~~~~~~~~~~~

The |ProductName| ecosystem also offers the following desktop applications:

* The `SIL Kit Dashboard <https://www.vector.com/SIL-Kit-Dashboard/>`_  collects, persists 
  and displays information from different SIL Kit systems.


.. _sec:overview-guiding-principles:

Guiding Principles
------------------

|ProductName| follows a few fundamental principles:

* **Local view:** Participants do not require knowledge about the overall system setup or other participants.
  Communication between participants is made possible by using a common bus network or topic name.
* **Distributed simulation:** Communication between participants is based on a peer-to-peer connections.
  There is no central simulation participant that handles the distribution of messages or controls the virtual time.
* **Stability:** |ProductName| has a stable network layer and provides API and ABI guarantees.
  This means that participants of different versions can partake in the same simulation.
  Further, the |ProductName| library can be updated to newer versions without having to adapt or recompile already existing |ProductName| applications.
* **Reconfigurability:** Users should be able to provide a configuration file to the SIL Kit application to change the its behavior (e.g., configure logging or change the network a bus controller connects to).

.. _sec:overview-terminology:

Terminology
-----------

.. list-table::
 :widths: 20 80

 * - (Distributed) Simulation Setup
   - A simulation setup consists of interconnected participants and a |ProductName| registry instance.
     The participants and the registry can be distributed in a network or running on the same host.
 * - :doc:`(Simulation) Participant<../api/participant>`
   - A participant is a communication node in the distributed simulation that provides access to the |ProductName| services.
 * - :ref:`Services<sec:api-services>`
   - Participants interact with each other through the means of services.
     These can be communication services, e.g., a :doc:`CAN Controller</api/services/can>` and orchestration services, such as the the Lifecycle Service, Time Synchronization Service, and the Logging Service.
 * - :doc:`Configuration<../configuration/configuration>`
   - The optional participant configuration file allows easy configuration of a participant and its interconnection within the simulation.
     It can be used to change a participant's behavior without recompiling its sources.
 * - Registry
   - The registry is a central process that enables participant discovery in a distributed simulation.
     An instance of the registry is required for coordination, either as a standalone process (see :ref:`sec:util-registry`) or created programmatically.
 * - Registry URI
   - The registry's URI specifies where the registry can be reached.
     It defaults to ``silkit://localhost:8500``, that is, the registry is reachable via TCP/IP on the 'localhost' on port 8500.
 * - :doc:`Middleware<../configuration/middleware-configuration>`
   - The concrete distributed communication implementation.
     That is, the software layer implementing the distributed message passing mechanism.
 * - Network Connection
   - All participants in a |ProductName| simulation communicate through peer-to-peer connections.
     By default, participants try to establish a connection via Unix Domain Sockets or TCP/IP.
     If none of these work, they try to communicate by using the |ProductName| Registry as a proxy.
 * - :ref:`(Virtual) Simulation Time <sec:sim-synchronization>`
   - Participants can opt to use a virtual simulation time that is synchronized with other participants.
 * - Required participant
   - A participant that is required for a successful simulation.
     All required participants impact the system state (see :ref:`Lifecycle Coordination Between Participants<sec:sim-lifecycle-syncParticipants>`).
     Required participants are defined by the :ref:`System Controller Utility<sec:util-system-controller>`.
 * - Coordinated participant
   - Participants can opt to synchronize their lifecycle state with other participants.
     Coordinated participants are subject to the state of all required participants.
 * - Autonomous participant
   - Participants can opt to make their participant state known to others but to ignore the system state.
     Autonomous participants are not stopped by the system state and therefore must be stopped manually.


Further Reading
---------------

More real-world examples involving time synchronization and simulated automotive networks, can be found in the :doc:`API sections<../api/api>`.
Also, studying the source code of the bundled :doc:`demo applications<../demos/demos>` is a good start.
The simulation lifecycle and supported simulation time synchronization are discussed in :doc:`../simulation/simulation`.
Additionally, :doc:`../configuration/configuration` describes how the participant configuration file can be used to change the behavior of participants.


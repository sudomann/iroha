/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <thread>

#include <gtest/gtest.h>
#include <boost/thread/barrier.hpp>
#include <boost/noncopyable.hpp>
#include "framework/integration_framework/port_guard.hpp"

using PortGuard = integration_framework::PortGuard;

TEST(PortGuardTest, AllPortsGetUsedAndNoOverlaps) {
  static constexpr size_t kNumClients = 10; // first-run clients
  static constexpr size_t kNumClientsKilled = 3; // then these die
  static constexpr size_t kNumClientsResurrected = 5; // then these are added
  static constexpr std::pair<size_t, size_t> kRange = {123, 456};

  struct Client : boost::noncopyable {
    Client() {};
    Client(Client &&other)
        : port_guard(std::move(other.port_guard)),
          used_ports(std::move(other.used_ports)){};

    PortGuard port_guard;
    std::bitset<PortGuard::kMaxPort + 1> used_ports;
  };

  std::vector<Client> clients;

  // start the given amount of client threads and wait till they complete
  auto run_new_clients = [&clients](const size_t num_clients) {
    boost::barrier bar(num_clients);
    auto port_requester = [&bar, &clients](
                              const size_t client_number) {
      Client &client = clients[client_number];
      bar.wait();
      while (true) {
        auto port = client.port_guard.tryGetPort(kRange.first, kRange.second);
        if (!port) {
          break;
        }
        client.used_ports.set(*port);
      }
    };

    clients.reserve(clients.size() + num_clients);
    std::vector<std::thread> threads;
    threads.reserve(num_clients);
    for (size_t client_number = 0; client_number < num_clients;
         ++client_number) {
      clients.emplace_back();
      threads.emplace_back(port_requester, clients.size() - 1);
    }

    // wait for the threads to complete
    for (auto &thread : threads) {
      thread.join();
    }
  };

  // check that all the ports in the interval are taken and none intersect
  auto check = [&clients] {
    std::bitset<PortGuard::kMaxPort + 1> all_used_ports;
    for (auto &client : clients) {
      ASSERT_TRUE((all_used_ports & client.used_ports).none())
          << "Some ports were used by more than one client!";
      all_used_ports |= client.used_ports;
    }
    for (size_t port = kRange.first; port <= kRange.second; ++port) {
      ASSERT_TRUE(all_used_ports.test(port))
          << "Port " << port << " was not given out to any client!";
    }
  };

  // first run
  run_new_clients(kNumClients);
  check();

  // kill some clients...
  clients.resize(kNumClients - kNumClientsKilled);
  // ... and run some new clients
  run_new_clients(kNumClientsResurrected);
  check();
}

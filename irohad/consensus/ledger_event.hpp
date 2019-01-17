/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef LEDGER_EVENT_HPP
#define LEDGER_EVENT_HPP

#include <memory>

#include "interfaces/common_objects/peer.hpp"

namespace iroha {
  using PeerList = std::vector<std::shared_ptr<shared_model::interface::Peer>>;

  struct LedgerEvent {
    std::unique_ptr<PeerList> ledger_peers;
  };
}  // namespace iroha

#endif  // LEDGER_EVENT_HPP

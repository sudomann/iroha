/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "ametsuchi/impl/in_memory_block_storage.hpp"

#include "interfaces/iroha_internal/block.hpp"

using namespace iroha::ametsuchi;

bool InMemoryBlockStorage::insert(Identifier id,
                                  const shared_model::interface::Block &block) {
  return block_store_.emplace(id, clone(block)).second;
}

boost::optional<std::shared_ptr<shared_model::interface::Block>>
InMemoryBlockStorage::fetch(Identifier id) const {
  auto it = block_store_.find(id);
  if (it != block_store_.end()) {
    return it->second;
  } else {
    return boost::none;
  }
}

InMemoryBlockStorage::Identifier InMemoryBlockStorage::lastId() const {
  return block_store_.rbegin()->first;
}

void InMemoryBlockStorage::dropAll() {
  block_store_.clear();
}

void InMemoryBlockStorage::visit(
    std::function<void(Identifier,
                       std::shared_ptr<shared_model::interface::Block>)>
        visitor) const {
  for (const auto &pair : block_store_) {
    visitor(pair.first, pair.second);
  }
}

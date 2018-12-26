/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_BLOCK_STORAGE_HPP
#define IROHA_BLOCK_STORAGE_HPP

#include <cstdint>
#include <functional>
#include <memory>

#include <boost/optional.hpp>

namespace shared_model {
  namespace interface {
    class Block;
  }  // namespace interface
}  // namespace shared_model

namespace iroha {
  namespace ametsuchi {

    /**
     * Append-only block storage interface
     */
    class BlockStorage {
     public:
      /**
       * Type of storage key
       */
      using Identifier = uint32_t;

      /**
       * Append block with given key
       * @return true if inserted successfully, false otherwise
       */
      virtual bool insert(Identifier id,
                          const shared_model::interface::Block &block) = 0;

      /**
       * Get block associated with given identifier
       * @return block if exists, boost::none otherwise
       */
      virtual boost::optional<std::shared_ptr<shared_model::interface::Block>>
      fetch(Identifier id) const = 0;

      /**
       * Returns last appended key
       */
      virtual Identifier lastId() const = 0;

      /**
       * Clears the contents of storage
       */
      virtual void dropAll() = 0;

      /**
       * Iterates through all the stored blocks
       */
      virtual void visit(
          std::function<void(Identifier,
                             std::shared_ptr<shared_model::interface::Block>)>
              visitor) const = 0;

      virtual ~BlockStorage() = default;
    };

  }  // namespace ametsuchi
}  // namespace iroha

#endif  // IROHA_BLOCK_STORAGE_HPP

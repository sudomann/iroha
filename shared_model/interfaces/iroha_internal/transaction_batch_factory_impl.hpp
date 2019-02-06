/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_TRANSACTION_BATCH_FACTORY_IMPL_HPP
#define IROHA_TRANSACTION_BATCH_FACTORY_IMPL_HPP

#include "interfaces/iroha_internal/transaction_batch_factory.hpp"
#include "validators/transaction_batch_validator.hpp"

namespace shared_model {
  namespace interface {

    class TransactionBatchFactoryImpl : public TransactionBatchFactory {
     public:
      using FactoryImplResult =
          FactoryResult<std::unique_ptr<TransactionBatch>>;

      FactoryImplResult createTransactionBatch(
          types::SharedTxsCollectionType transactions) const override;

      FactoryImplResult createTransactionBatch(
          std::shared_ptr<Transaction> transaction) const override;

     private:
      validation::BatchValidator batch_validator_;
    };

  }  // namespace interface
}  // namespace shared_model

#endif  // IROHA_TRANSACTION_BATCH_FACTORY_IMPL_HPP

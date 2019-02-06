/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "interfaces/iroha_internal/transaction_batch_factory_impl.hpp"

#include "interfaces/iroha_internal/transaction_batch_impl.hpp"
#include "interfaces/transaction.hpp"
#include "validators/answer.hpp"

namespace shared_model {
  namespace interface {
    TransactionBatchFactoryImpl::FactoryImplResult
    TransactionBatchFactoryImpl::createTransactionBatch(
        types::SharedTxsCollectionType transactions) const {
      std::unique_ptr<TransactionBatch> batch_ptr =
          std::make_unique<TransactionBatchImpl>(std::move(transactions));
      if (auto answer = batch_validator_.validate(*batch_ptr)) {
        return iroha::expected::makeError(answer.reason());
      }
      return iroha::expected::makeValue(std::move(batch_ptr));
    }

    TransactionBatchFactoryImpl::FactoryImplResult
    TransactionBatchFactoryImpl::createTransactionBatch(
        std::shared_ptr<Transaction> transaction) const {
      return createTransactionBatch(
          types::SharedTxsCollectionType{std::move(transaction)});
    }
  }  // namespace interface
}  // namespace shared_model

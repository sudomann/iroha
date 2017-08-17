/**
 * Copyright Soramitsu Co., Ltd. 2017 All Rights Reserved.
 * http://soramitsu.co.jp
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <iostream>
#include <model/tx_responses/stateless_response.hpp>
#include <torii/processor/transaction_processor_impl.hpp>
#include <utility>

namespace iroha {
  namespace torii {

    using validation::StatelessValidator;
    using model::TransactionResponse;
    using network::PeerCommunicationService;

    TransactionProcessorImpl::TransactionProcessorImpl(
        std::shared_ptr<PeerCommunicationService> pcs,
        std::shared_ptr<StatelessValidator> validator)
        : pcs_(std::move(pcs)), validator_(std::move(validator)) {
      log_ = logger::log("TxProcessor");

      // insert all txs from proposal to proposal set
      pcs_->on_proposal().subscribe([this](model::Proposal proposal) {
        for (auto tx : proposal.transactions) {
          proposal_set_.insert(tx.tx_hash.to_string());
          TransactionResponse response;
          response.tx_hash = tx.tx_hash.to_string();
          response.current_status =
              TransactionResponse::STATELESS_VALIDATION_SUCCESS;
        }
      });

      // move commited txs from proposal to candidate map
      pcs_->on_commit().subscribe(
          [this](rxcpp::observable<model::Block> blocks) {
            blocks.subscribe(
                // on next..
                [this](model::Block block) {
                  for (auto tx : block.transactions) {
                    if (this->proposal_set_.count(tx.tx_hash.to_string())) {
                      candidate_set_.insert(tx.tx_hash.to_string());
                    }
                  }
                },
                // on complete
                [this]() {
                  for (auto tx_hash : proposal_set_) {
                    TransactionResponse response;
                    response.tx_hash = tx_hash;
                    response.current_status =
                        TransactionResponse::STATEFUL_VALIDATION_FAILED;
                  }
                  proposal_set_.clear();

                  for (auto tx_hash : candidate_set_) {
                    TransactionResponse response;
                    response.tx_hash = tx_hash;
                    response.current_status = TransactionResponse::COMMITTED;
                  }
                  candidate_set_.clear();
                });
          });
    }

    void TransactionProcessorImpl::transactionHandle(
        std::shared_ptr<model::Transaction> transaction) {
      log_->info("handle transaction");
      model::TransactionStatelessResponse response;
      response.tx_hash = transaction->tx_hash.to_string();
      response.passed = false;

      if (validator_->validate(*transaction)) {
        response.passed = true;
        pcs_->propagate_transaction(transaction);
      }
      log_->info("stateless validation status: {}", response.passed);
      notifier_.get_subscriber().on_next(
          std::make_shared<model::TransactionStatelessResponse>(response));
    }

    rxcpp::observable<std::shared_ptr<model::TransactionResponse>>
    TransactionProcessorImpl::transactionNotifier() {
      return notifier_.get_observable();
    }

  }  // namespace torii
}  // namespace iroha

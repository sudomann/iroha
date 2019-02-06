/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_ON_DEMAND_ORDERING_SERVICE_IMPL_HPP
#define IROHA_ON_DEMAND_ORDERING_SERVICE_IMPL_HPP

#include "ordering/on_demand_ordering_service.hpp"

#include <queue>
#include <shared_mutex>
#include <unordered_map>

#include <tbb/concurrent_queue.h>
#include "interfaces/iroha_internal/unsafe_proposal_factory.hpp"
#include "logger/logger.hpp"
#include "ordering/impl/on_demand_common.hpp"

namespace iroha {
  namespace ametsuchi {
    class TxPresenceCache;
  }
  namespace ordering {
    class OnDemandOrderingServiceImpl : public OnDemandOrderingService {
     public:
      /**
       * Create on_demand ordering service with following options:
       * @param transaction_limit - number of maximum transactions in one
       * proposal
       * @param proposal_factory - used to generate proposals
       * @param tx_cache - cache of transactions
       * @param number_of_proposals - number of stored proposals, older will be
       * removed. Default value is 3
       * @param initial_round - first round of agreement.
       * Default value is {2, kFirstRejectRound} since genesis block height is 1
       * @param log to print progress
       */
      OnDemandOrderingServiceImpl(
          size_t transaction_limit,
          std::shared_ptr<shared_model::interface::UnsafeProposalFactory>
              proposal_factory,
          std::shared_ptr<ametsuchi::TxPresenceCache> tx_cache,
          size_t number_of_proposals = 3,
          const consensus::Round &initial_round = {2, kFirstRejectRound},
          logger::Logger log = logger::log("OnDemandOrderingServiceImpl"));

      // --------------------- | OnDemandOrderingService |_---------------------

      void onCollaborationOutcome(consensus::Round round) override;

      // ----------------------- | OdOsNotification | --------------------------

      void onBatches(consensus::Round, CollectionType batches) override;

      boost::optional<ProposalType> onRequestProposal(
          consensus::Round round) override;

     private:
      /**
       * Packs new proposals and creates new rounds
       * Note: method is not thread-safe
       */
      void packNextProposals(const consensus::Round &round);

      /**
       * Removes last elements if it is required
       * Method removes the oldest commit or chain of the oldest rejects
       * Note: method is not thread-safe
       */
      void tryErase();

      /**
       * Get transactions from the given batches queue. Does not break batches -
       * continues getting all the transactions from the ongoing batch until the
       * required amount is collected.
       * @param requested_tx_amount - amount of transactions to get
       * @param tx_batches_queue - the queue to get transactions from
       * @return transactions
       */
      static std::vector<std::shared_ptr<shared_model::interface::Transaction>>
      getTransactions(
          size_t requested_tx_amount,
          tbb::concurrent_queue<TransactionBatchType> tx_batches_queue);

      /**
       * Check if batch was already processed by the peer
       */
      bool batchAlreadyProcessed(
          const shared_model::interface::TransactionBatch &batch);

      /**
       * Max number of transaction in one proposal
       */
      size_t transaction_limit_;

      /**
       * Max number of available proposals in one OS
       */
      size_t number_of_proposals_;

      /**
       * Queue which holds all rounds in linear order
       */
      std::queue<consensus::Round> round_queue_;

      /**
       * Map of available proposals
       */
      std::unordered_map<consensus::Round,
                         ProposalType,
                         consensus::RoundTypeHasher>
          proposal_map_;

      /**
       * Proposals for current rounds
       */
      std::unordered_map<consensus::Round,
                         tbb::concurrent_queue<TransactionBatchType>,
                         consensus::RoundTypeHasher>
          current_proposals_;

      /**
       * Read write mutex for public methods
       */
      std::shared_timed_mutex lock_;

      std::shared_ptr<shared_model::interface::UnsafeProposalFactory>
          proposal_factory_;

      /**
       * Processed transactions cache used for replay prevention
       */
      std::shared_ptr<ametsuchi::TxPresenceCache> tx_cache_;

      /**
       * Logger instance
       */
      logger::Logger log_;
    };
  }  // namespace ordering
}  // namespace iroha

#endif  // IROHA_ON_DEMAND_ORDERING_SERVICE_IMPL_HPP

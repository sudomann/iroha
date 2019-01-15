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

#include "consensus/yac/storage/yac_vote_storage.hpp"

#include <algorithm>
#include <utility>

#include "common/bind.hpp"
#include "consensus/yac/storage/yac_proposal_storage.hpp"

namespace iroha {
  namespace consensus {
    namespace yac {

      // --------| private api |--------

      auto YacVoteStorage::getProposalStorage(const Round &round) {
        return std::find_if(proposal_storages_.begin(),
                            proposal_storages_.end(),
                            [&round](const auto &storage) {
                              return storage.getStorageKey() == round;
                            });
      }

      boost::optional<std::vector<YacProposalStorage>::iterator>
      YacVoteStorage::findProposalStorage(const VoteMessage &msg,
                                          PeersNumberType peers_in_round) {
        const auto &round = msg.hash.vote_round;
        auto val = getProposalStorage(round);
        if (val != proposal_storages_.end()) {
          return val;
        }
        if (strategy_->shouldCreateRound(round)) {
          return proposal_storages_.emplace(
              proposal_storages_.end(),
              msg.hash.vote_round,
              peers_in_round,
              std::make_shared<SupermajorityCheckerImpl>());
        } else {
          return boost::none;
        }
      }

      void YacVoteStorage::removeByRound(const iroha::consensus::Round &round) {
        auto val = getProposalStorage(round);
        if (val != proposal_storages_.end()) {
          proposal_storages_.erase(val);
        }
      }

      // --------| public api |--------

      YacVoteStorage::YacVoteStorage(
          std::shared_ptr<CleanupStrategy> cleanup_strategy)
          : strategy_(std::move(cleanup_strategy)) {}

      boost::optional<Answer> YacVoteStorage::store(
          std::vector<VoteMessage> state, PeersNumberType peers_in_round) {
        Round r;
        return findProposalStorage(state.at(0), peers_in_round) |
                   [&state, &r](auto &&storage) {
                     r = storage->getStorageKey();
                     return storage->insert(state);
                   }
                   | [this,
                      &r](auto &&insert_outcome) -> boost::optional<Answer> {
          this->strategy_->finalize(r, insert_outcome) | [this](auto &&remove) {
            std::for_each(
                remove.begin(), remove.end(), [this](const auto &round) {
                  this->removeByRound(round);
                });
          };
          return insert_outcome;
        };
      }

      bool YacVoteStorage::isCommitted(const Round &round) {
        auto iter = getProposalStorage(round);
        if (iter == proposal_storages_.end()) {
          return false;
        }
        return bool(iter->getState());
      }

      ProposalState YacVoteStorage::getProcessingState(const Round &round) {
        return processing_state_[round];
      }

      void YacVoteStorage::nextProcessingState(const Round &round) {
        auto &val = processing_state_[round];
        switch (val) {
          case ProposalState::kNotSentNotProcessed:
            val = ProposalState::kSentNotProcessed;
            break;
          case ProposalState::kSentNotProcessed:
            val = ProposalState::kSentProcessed;
            break;
          case ProposalState::kSentProcessed:
            break;
        }
      }

    }  // namespace yac
  }    // namespace consensus
}  // namespace iroha

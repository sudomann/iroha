/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "common/default_constructible_unary_fn.hpp"  // non-copyable value workaround

#include "torii/impl/command_service_transport_grpc.hpp"

#include <atomic>
#include <iterator>

#include <boost/format.hpp>
#include <boost/range/adaptor/filtered.hpp>
#include <boost/range/adaptor/transformed.hpp>
#include "backend/protobuf/transaction_responses/proto_tx_response.hpp"
#include "interfaces/iroha_internal/transaction_batch.hpp"
#include "interfaces/iroha_internal/transaction_batch_factory.hpp"
#include "interfaces/iroha_internal/transaction_batch_parser.hpp"
#include "interfaces/iroha_internal/tx_status_factory.hpp"
#include "interfaces/transaction.hpp"
#include "torii/status_bus.hpp"

namespace torii {

  CommandServiceTransportGrpc::CommandServiceTransportGrpc(
      std::shared_ptr<CommandService> command_service,
      std::shared_ptr<iroha::torii::StatusBus> status_bus,
      std::shared_ptr<shared_model::interface::TxStatusFactory> status_factory,
      std::shared_ptr<TransportFactoryType> transaction_factory,
      std::shared_ptr<shared_model::interface::TransactionBatchParser>
          batch_parser,
      std::shared_ptr<shared_model::interface::TransactionBatchFactory>
          transaction_batch_factory,
      rxcpp::observable<ConsensusGateEvent> consensus_gate_objects,
      int maximum_rounds_without_update,
      logger::Logger log)
      : command_service_(std::move(command_service)),
        status_bus_(std::move(status_bus)),
        status_factory_(std::move(status_factory)),
        transaction_factory_(std::move(transaction_factory)),
        batch_parser_(std::move(batch_parser)),
        batch_factory_(std::move(transaction_batch_factory)),
        log_(std::move(log)),
        consensus_gate_objects_(std::move(consensus_gate_objects)),
        maximum_rounds_without_update_(maximum_rounds_without_update) {}

  grpc::Status CommandServiceTransportGrpc::Torii(
      grpc::ServerContext *context,
      const iroha::protocol::Transaction *request,
      google::protobuf::Empty *response) {
    iroha::protocol::TxList single_tx_list;
    *single_tx_list.add_transactions() = *request;
    return ListTorii(context, &single_tx_list, response);
  }

  namespace {
    /**
     * Form an error message, which is to be shared between all transactions,
     * if there are several of them, or individual message, if there's only
     * one
     * @param tx_hashes is non empty hash list to form error message from
     * @param error of those tx(s)
     * @return message
     */
    std::string formErrorMessage(
        const std::vector<shared_model::crypto::Hash> &tx_hashes,
        const std::string &error) {
      if (tx_hashes.size() == 1) {
        return (boost::format("Stateless invalid tx, error: %s, hash: %s")
                % error % tx_hashes[0].hex())
            .str();
      }

      std::string folded_hashes =
          std::accumulate(std::next(tx_hashes.begin()),
                          tx_hashes.end(),
                          tx_hashes[0].hex(),
                          [](auto &&acc, const auto &h) -> std::string {
                            return acc + ", " + h.hex();
                          });

      return (boost::format(
                  "Stateless invalid tx in transaction sequence, error: %s\n"
                  "Hash list: [%s]")
              % error % folded_hashes)
          .str();
    }
  }  // namespace

  shared_model::interface::types::SharedTxsCollectionType
  CommandServiceTransportGrpc::deserializeTransactions(
      const iroha::protocol::TxList *request) {
    shared_model::interface::types::SharedTxsCollectionType tx_collection;
    for (const auto &tx : request->transactions()) {
      transaction_factory_->build(tx).match(
          [&tx_collection](
              iroha::expected::Value<
                  std::unique_ptr<shared_model::interface::Transaction>> &v) {
            tx_collection.emplace_back(std::move(v).value);
          },
          [this](iroha::expected::Error<TransportFactoryType::Error> &error) {
            status_bus_->publish(status_factory_->makeStatelessFail(
                error.error.hash,
                shared_model::interface::TxStatusFactory::TransactionError{
                    error.error.error, 0, 0}));
          });
    }
    return tx_collection;
  }

  grpc::Status CommandServiceTransportGrpc::ListTorii(
      grpc::ServerContext *context,
      const iroha::protocol::TxList *request,
      google::protobuf::Empty *response) {
    auto transactions = deserializeTransactions(request);

    auto batches = batch_parser_->parseBatches(transactions);

    for (auto &batch : batches) {
      batch_factory_->createTransactionBatch(batch).match(
          [&](iroha::expected::Value<std::unique_ptr<
                  shared_model::interface::TransactionBatch>> &value) {
            this->command_service_->handleTransactionBatch(
                std::move(value).value);
          },
          [&](iroha::expected::Error<std::string> &error) {
            std::vector<shared_model::crypto::Hash> hashes;

            std::transform(batch.begin(),
                           batch.end(),
                           std::back_inserter(hashes),
                           [](const auto &tx) { return tx->hash(); });

            auto error_msg = formErrorMessage(hashes, error.error);
            // set error response for each transaction in a batch candidate
            std::for_each(
                hashes.begin(), hashes.end(), [this, &error_msg](auto &hash) {
                  status_bus_->publish(status_factory_->makeStatelessFail(
                      hash,
                      shared_model::interface::TxStatusFactory::
                          TransactionError{error_msg, 0, 0}));
                });
          });
    }

    return grpc::Status::OK;
  }

  grpc::Status CommandServiceTransportGrpc::Status(
      grpc::ServerContext *context,
      const iroha::protocol::TxStatusRequest *request,
      iroha::protocol::ToriiResponse *response) {
    *response =
        std::static_pointer_cast<shared_model::proto::TransactionResponse>(
            command_service_->getStatus(
                shared_model::crypto::Hash::fromHexString(request->tx_hash())))
            ->getTransport();
    return grpc::Status::OK;
  }

  namespace {
    void handleEvents(rxcpp::composite_subscription &subscription,
                      rxcpp::schedulers::run_loop &run_loop) {
      while (subscription.is_subscribed() or not run_loop.empty()) {
        run_loop.dispatch();
      }
    }
  }  // namespace

  grpc::Status CommandServiceTransportGrpc::StatusStream(
      grpc::ServerContext *context,
      const iroha::protocol::TxStatusRequest *request,
      grpc::ServerWriter<iroha::protocol::ToriiResponse> *response_writer) {
    rxcpp::schedulers::run_loop rl;

    auto current_thread =
        rxcpp::synchronize_in_one_worker(rxcpp::schedulers::make_run_loop(rl));

    rxcpp::composite_subscription subscription;

    auto hash = shared_model::crypto::Hash::fromHexString(request->tx_hash());

    auto client_id_format = boost::format("Peer: '%s', %s");
    std::string client_id =
        (client_id_format % context->peer() % hash.toString()).str();
    bool last_tx_status_received = false;
    auto status_bus = command_service_->getStatusStream(hash)
                          .finally([&last_tx_status_received] {
                            last_tx_status_received = true;
                          })
                          .publish()
                          .ref_count();
    auto consensus_gate_observable =
        consensus_gate_objects_
            // a dummy start_with lets us don't wait for the consensus event
            // on further combine_latest
            .start_with(ConsensusGateEvent{})
            .publish()
            .ref_count();

    boost::optional<iroha::protocol::TxStatus> last_tx_status;
    auto rounds_counter{0};
    status_bus
        // convert to transport objects
        .map([&](auto response) {
          log_->info("mapped {}, {}", *response, client_id);
          return std::static_pointer_cast<
                     shared_model::proto::TransactionResponse>(response)
              ->getTransport();
        })
        .combine_latest(current_thread, consensus_gate_observable)
        .map([](const auto &tuple) { return std::get<0>(tuple); })
        // complete the observable if client is disconnected or too many
        // rounds have passed without tx status change
        .take_while([=,
                     &rounds_counter,
                     &last_tx_status,
                     // last_tx_status_received has to be passed by reference to
                     // prevent accessing its outdated state
                     &last_tx_status_received](const auto &response) {
          if (context->IsCancelled()) {
            log_->debug("client unsubscribed, {}", client_id);
            return false;
          }

          // increment round counter when the same status arrived again.
          auto status = response.tx_status();
          auto status_is_same = last_tx_status and (status == *last_tx_status);
          if (status_is_same) {
            ++rounds_counter;
            if (rounds_counter >= maximum_rounds_without_update_) {
              // we stop the stream when round counter is greater than allowed.
              return false;
            }
            // omit the received status, but do not stop the stream
            return true;
          }
          rounds_counter = 0;
          last_tx_status = status;

          // write a new status to the stream
          if (not response_writer->Write(response)) {
            log_->error("write to stream has failed to client {}", client_id);
            return false;
          }

          log_->debug("status written, {}", client_id);
          if (last_tx_status_received) {
            // force stream to end because no more tx statuses will arrive.
            // it is thread safe because of synchronization on current_thread
            return false;
          }
          return true;
        })
        .subscribe(subscription,
                   [](const auto &) {},
                   [&](std::exception_ptr ep) {
                     log_->error("something bad happened, client_id {}",
                                 client_id);
                   },
                   [&] { log_->debug("stream done, {}", client_id); });

    // run loop while subscription is active or there are pending events in
    // the queue
    handleEvents(subscription, rl);

    log_->debug("status stream done, {}", client_id);
    return grpc::Status::OK;
  }  // namespace torii
}  // namespace torii

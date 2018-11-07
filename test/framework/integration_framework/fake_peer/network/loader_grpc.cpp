/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "framework/integration_framework/fake_peer/network/loader_grpc.hpp"

#include "backend/protobuf/block.hpp"
#include "framework/integration_framework/fake_peer/behaviour/behaviour.hpp"
#include "framework/integration_framework/fake_peer/fake_peer.hpp"

namespace integration_framework {
  namespace fake_peer {

    LoaderGrpc::LoaderGrpc(const std::shared_ptr<FakePeer> &fake_peer)
        : fake_peer_wptr_(fake_peer) {}

    ::grpc::Status LoaderGrpc::retrieveBlock(
        ::grpc::ServerContext *context,
        const iroha::network::proto::BlockRequest *request,
        iroha::protocol::Block *response) {
      LoaderBlockRequest hash =
          std::make_shared<shared_model::crypto::Hash>(request->hash());
      auto fake_peer = fake_peer_wptr_.lock();
      BOOST_VERIFY_MSG(fake_peer, "Fake peer shared pointer is not set!");
      auto behaviour = fake_peer->getBehaviour();
      if (!behaviour) {
        return ::grpc::Status(::grpc::StatusCode::INTERNAL,
                              "Fake Peer has no behaviour set!");
      }
      auto block = behaviour->processLoaderBlockRequest(hash);
      if (!block) {
        return ::grpc::Status(::grpc::StatusCode::NOT_FOUND, "Block not found");
      }
      *response = block->getTransport();
      return ::grpc::Status::OK;
    }

    ::grpc::Status LoaderGrpc::retrieveBlocks(
        ::grpc::ServerContext *context,
        const iroha::network::proto::BlocksRequest *request,
        ::grpc::ServerWriter<iroha::protocol::Block> *writer) {
      LoaderBlocksRequest height = request->height();
      auto fake_peer = fake_peer_wptr_.lock();
      BOOST_VERIFY_MSG(fake_peer, "Fake peer shared pointer is not set!");
      auto behaviour = fake_peer->getBehaviour();
      if (!behaviour) {
        return ::grpc::Status(::grpc::StatusCode::INTERNAL,
                              "Fake Peer has no behaviour set!");
      }
      auto blocks = behaviour->processLoaderBlocksRequest(height);
      if (blocks.size() == 0) {
        return ::grpc::Status(::grpc::StatusCode::NOT_FOUND, "Block not found");
      }
      for (auto &block : blocks) {
        writer->Write(block.get().getTransport());
      }
      return ::grpc::Status::OK;
    }

  }  // namespace fake_peer
}  // namespace integration_framework

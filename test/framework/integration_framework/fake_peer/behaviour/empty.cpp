#include "framework/integration_framework/fake_peer/behaviour/empty.hpp"

namespace integration_framework {
  namespace fake_peer {

    void EmptyBehaviour::processMstMessage(
        const FakePeer::MstMessagePtr &message) {}
    void EmptyBehaviour::processYacMessage(
        const FakePeer::YacMessagePtr &message) {}
    void EmptyBehaviour::processOsBatch(const FakePeer::OsBatchPtr &batch) {}
    void EmptyBehaviour::processOgProposal(
        const FakePeer::OgProposalPtr &proposal) {}
    EmptyBehaviour::LoaderBlockRequestResult
    EmptyBehaviour::processLoaderBlockRequest(
        const FakePeer::LoaderBlockRequest &request) {
      return {};
    }
    EmptyBehaviour::LoaderBlocksRequestResult
    EmptyBehaviour::processLoaderBlocksRequest(
        const FakePeer::LoaderBlocksRequest &request) {
      return {};
    }

    std::string EmptyBehaviour::getName() {
      return "empty behaviour";
    }

  }  // namespace fake_peer
}  // namespace integration_framework

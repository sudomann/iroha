#include "framework/integration_framework/fake_peer/behaviour/empty.hpp"

namespace integration_framework {

  void EmptyFakePeerBehaviour::processMstMessage(
      const FakePeer::MstMessagePtr &message) {}
  void EmptyFakePeerBehaviour::processYacMessage(
      const FakePeer::YacMessagePtr &message) {}
  void EmptyFakePeerBehaviour::processOsBatch(
      const FakePeer::OsBatchPtr &batch) {}
  void EmptyFakePeerBehaviour::processOgProposal(
      const FakePeer::OgProposalPtr &proposal) {}

  std::string EmptyFakePeerBehaviour::getName() {
    return "empty behaviour";
  }

}  // namespace integration_framework

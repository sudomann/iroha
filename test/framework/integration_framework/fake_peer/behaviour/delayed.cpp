#include "framework/integration_framework/fake_peer/behaviour/delayed.hpp"

namespace integration_framework {

  DelayedFakePeerBehaviour::DelayedFakePeerBehaviour(
      FakePeerBehaviour &base_behaviour, std::chrono::milliseconds delay)
      : FakePeerBehaviourDecorator(base_behaviour), delay_(delay) {}

  void DelayedFakePeerBehaviour::processMstMessage(
      const FakePeer::MstMessagePtr &message) {
    std::this_thread::sleep_for(delay_);
    base_behaviour_.processMstMessage(message);
  }

  void DelayedFakePeerBehaviour::processYacMessage(
      const FakePeer::YacMessagePtr &message) {
    std::this_thread::sleep_for(delay_);
    base_behaviour_.processYacMessage(message);
  }

  void DelayedFakePeerBehaviour::processOsBatch(
      const FakePeer::OsBatchPtr &batch) {
    std::this_thread::sleep_for(delay_);
    base_behaviour_.processOsBatch(batch);
  }

  void DelayedFakePeerBehaviour::processOgProposal(
      const FakePeer::OgProposalPtr &proposal) {
    std::this_thread::sleep_for(delay_);
    base_behaviour_.processOgProposal(proposal);
  }

  std::string DelayedFakePeerBehaviour::getName() {
    return "delayed " + base_behaviour_.getName();
  }

}  // namespace integration_framework

#include "framework/integration_framework/fake_peer/behaviour/delayed.hpp"

namespace integration_framework {
  namespace fake_peer {

    DelayedBehaviour::DelayedBehaviour(Behaviour &base_behaviour,
                                       std::chrono::milliseconds delay)
        : BehaviourDecorator(base_behaviour), delay_(delay) {}

    void DelayedBehaviour::processMstMessage(
        const FakePeer::MstMessagePtr &message) {
      std::this_thread::sleep_for(delay_);
      base_behaviour_.processMstMessage(message);
    }

    void DelayedBehaviour::processYacMessage(
        const FakePeer::YacMessagePtr &message) {
      std::this_thread::sleep_for(delay_);
      base_behaviour_.processYacMessage(message);
    }

    void DelayedBehaviour::processOsBatch(const FakePeer::OsBatchPtr &batch) {
      std::this_thread::sleep_for(delay_);
      base_behaviour_.processOsBatch(batch);
    }

    void DelayedBehaviour::processOgProposal(
        const FakePeer::OgProposalPtr &proposal) {
      std::this_thread::sleep_for(delay_);
      base_behaviour_.processOgProposal(proposal);
    }

    DelayedBehaviour::LoaderBlockRequestResult
    DelayedBehaviour::processLoaderBlockRequest(
        const FakePeer::LoaderBlockRequest &request) {
      std::this_thread::sleep_for(delay_);
      return base_behaviour_.processLoaderBlockRequest(request);
    }

    DelayedBehaviour::LoaderBlocksRequestResult
    DelayedBehaviour::processLoaderBlocksRequest(
        const FakePeer::LoaderBlocksRequest &request) {
      std::this_thread::sleep_for(delay_);
      return base_behaviour_.processLoaderBlocksRequest(request);
    }

    std::string DelayedBehaviour::getName() {
      return "delayed " + base_behaviour_.getName();
    }

  }  // namespace fake_peer
}  // namespace integration_framework

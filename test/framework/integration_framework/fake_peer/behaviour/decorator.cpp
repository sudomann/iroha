#include "framework/integration_framework/fake_peer/behaviour/decorator.hpp"

namespace integration_framework {
  namespace fake_peer {

    BehaviourDecorator::BehaviourDecorator(Behaviour &base_behaviour)
        : base_behaviour_(base_behaviour) {}

  }  // namespace fake_peer
}  // namespace integration_framework

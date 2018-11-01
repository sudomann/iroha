#include "framework/integration_framework/fake_peer/behaviour/decorator.hpp"

namespace integration_framework {

  FakePeerBehaviourDecorator::FakePeerBehaviourDecorator(
      FakePeerBehaviour &base_behaviour)
      : base_behaviour_(base_behaviour) {}

}  // namespace integration_framework

#include "territorygame/controller/AvailableControllers.hpp"

#include "candidate/CandidateController.hpp"
#include "candidate/examples/BasicStateMachine.hpp"
#include "candidate/examples/RandomStateMachine.hpp"
#include "territorygame/controller/EnemyStateMachine.hpp"

namespace territorygame::controller {

const std::vector<ControllerOption> &availableControllers() {
  static const std::vector<ControllerOption> all{
      ControllerOption{
          "Basic State Machine",
          [](int64_t) {
            return std::make_shared<candidate::examples::BasicStateMachine>();
          }},
      ControllerOption{"Enemy State Machine",
                       [](int64_t seed) {
                         return std::make_shared<EnemyStateMachine>(seed);
                       }},
      ControllerOption{
          "Random State Machine",
          [](int64_t) {
            return std::make_shared<candidate::examples::RandomStateMachine>();
          }},
      ControllerOption{
          "Candidate Controller",
          [](int64_t) {
            return std::make_shared<candidate::CandidateController>();
          }},
  };
  return all;
}

} // namespace territorygame::controller

#pragma once

#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "territorygame/api/AgentController.hpp"

namespace territorygame::controller {

// Registry of controller implementations offered per player slot.
struct ControllerOption {
    std::string label;

    // The factory takes the player slot's configured random seed
    // (GameConfig::controllerSeeds); most controllers have no randomness of
    // their own and just ignore it.
    std::function<std::shared_ptr<territorygame::api::AgentController>(int64_t seed)> factory;
};

const std::vector<ControllerOption>& availableControllers();

} // namespace territorygame::controller

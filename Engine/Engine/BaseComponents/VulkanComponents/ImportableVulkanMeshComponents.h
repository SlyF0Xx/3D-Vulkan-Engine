#pragma once

#include "../../Engine.h"

#include <entt/entt.hpp>

#include <filesystem>

namespace diffusion {

void import_mesh(const std::filesystem::path& path, ::entt::registry& registry, ::entt::entity parent_entity);

} // namespace diffusion {
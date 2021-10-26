#pragma once

#include "Engine.h"

#include <entt/entt.hpp>

#include <filesystem>

namespace diffusion {

namespace entt {

::entt::entity import_entity(
	::entt::registry & registry,
	const std::filesystem::path& path,
	const glm::vec3& position,
	const glm::vec3& rotation,
	const glm::vec3& scale);

}

} // namespace diffusion {
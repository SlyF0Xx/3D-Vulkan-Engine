#pragma once

#include "../Engine.h"

#include <entt/entt.hpp>

#include <filesystem>

namespace diffusion {

::entt::entity import_entity(
	::entt::registry & registry,
	const std::filesystem::path& path,
	const glm::vec3& position = glm::vec3(0),
	const glm::vec3& rotation = glm::vec3(0),
	const glm::vec3& scale = glm::vec3(1));

} // namespace diffusion {
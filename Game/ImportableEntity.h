#pragma once

#include "Entity.h"
#include "Engine.h"

#include <filesystem>

namespace diffusion {

class ImportableEntity : public Entity
{
public:
	ImportableEntity(
		Game& game,
		const std::filesystem::path& path,
		const glm::vec3& position,
		const glm::vec3& rotation,
		const glm::vec3& scale);
	/*
	    ImportableVulkanMeshComponents(
        Game& game,
        const std::filesystem::path& path,
        const std::vector<Tag>& tags,
        Entity* parent);
	*/

    /*
    	VulkanTransformComponent(
		Game& game,
		const glm::vec3& position,
		const glm::vec3& rotation,
		const glm::vec3& scale,
		const std::vector<Tag>& tags,
		Entity* parent);
    */
};

} // namespace diffusion {
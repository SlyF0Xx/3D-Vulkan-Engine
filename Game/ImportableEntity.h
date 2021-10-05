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
};

class CatImportableEntity : public Entity
{
public:
	CatImportableEntity(
		Game& game,
		const std::filesystem::path& path,
		const glm::vec3& position,
		const glm::vec3& rotation,
		const glm::vec3& scale);

	static inline Component::Tag s_special_cat_transform_tag;
};

} // namespace diffusion {
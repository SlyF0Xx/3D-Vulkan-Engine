#pragma once

#include "Engine.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <entt/entt.hpp>

#include <filesystem>

namespace diffusion {

namespace entt {

void import_mesh(const std::filesystem::path& path, ::entt::registry& registry, ::entt::entity parent_entity);

}

} // namespace diffusion {
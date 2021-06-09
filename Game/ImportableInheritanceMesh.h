#pragma once

#include "Engine.h"

#include <glm/glm.hpp>

#include <filesystem>

class ImportableInheritanceMesh
{
public:
    ImportableInheritanceMesh(
        Game& game,
        const std::filesystem::path& path,
        const glm::vec3& position,
        const glm::vec3& rotation,
        const glm::vec3& scale,
        const glm::mat4& CameraMatrix,
        const glm::mat4& ProjectionMatrix);
};


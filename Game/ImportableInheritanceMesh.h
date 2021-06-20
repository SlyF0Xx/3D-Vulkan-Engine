#pragma once

#include "Engine.h"

#include "GameComponentMesh.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <glm/glm.hpp>

#include <filesystem>

class ImportableInheritanceMesh
{
private:
    Game& m_game;
    std::vector<GameComponentMesh> m_game_components;
    int m_start_material_index;

    int FillMeshes(
        const aiNode& root,
        const aiScene& scene,
        int index,
        int parent_index);

    void ImportChildNodes(
        const aiNode& root,
        const aiScene& scene,
        const glm::vec3& position,
        const glm::vec3& rotation,
        const glm::vec3& scale);

public:
    ImportableInheritanceMesh(
        Game& game,
        const std::filesystem::path& path,
        const glm::vec3& position,
        const glm::vec3& rotation,
        const glm::vec3& scale);
};


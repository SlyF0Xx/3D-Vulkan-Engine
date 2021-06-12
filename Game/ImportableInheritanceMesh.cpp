#include "ImportableInheritanceMesh.h"

#include "PrimitiveMesh.h"

#include <iostream>

int ImportableInheritanceMesh::FillMeshes(
    const aiNode& root,
    const aiScene& scene,
    int index,
    int parent_index)
{
    if (parent_index != -1) {
        m_game_components[index].set_parrent(&m_game_components[parent_index]);
    }

    for (int i = 0; i < root.mNumMeshes; ++i) {
        std::vector<PrimitiveColoredVertex> verticies;

        auto index = root.mMeshes[i];
        for (int j = 0; j < scene.mMeshes[index]->mNumVertices; ++j) {
            PrimitiveColoredVertex vertex;
            vertex.x = scene.mMeshes[index]->mVertices[j].x;
            vertex.y = scene.mMeshes[index]->mVertices[j].y;
            vertex.z = scene.mMeshes[index]->mVertices[j].z;
            vertex.color[0] = 0.7f;
            vertex.color[1] = 0.7f;
            vertex.color[2] = 0.7f;
            vertex.color[3] = 1.0f;
            verticies.push_back(vertex);
        }

        std::vector<uint32_t> indexes;
        for (int j = 0; j < scene.mMeshes[index]->mNumFaces; ++j) {
            for (int k = 0; k < scene.mMeshes[index]->mFaces[j].mNumIndices; ++k) {
                indexes.push_back(scene.mMeshes[index]->mFaces[j].mIndices[k]);
            }
        }

        // TODO: generate it!
        BoundingSphere bounding_sphere;
        m_game.register_mesh(0, new ImportableMesh(m_game, m_game_components[index], verticies, indexes, bounding_sphere));
    }

    int own_index = index;
    for (int i = 0; i < root.mNumChildren; ++i) {
        index = FillMeshes(*root.mChildren[i], scene, index + 1, own_index);
    }

    return index;
}

void ImportableInheritanceMesh::ImportChildNodes(
    const aiNode& root,
    const aiScene & scene,
    const glm::vec3& position,
    const glm::vec3& rotation,
    const glm::vec3& scale)
{
    GameComponentMesh game_component(
        m_game,
        position,
        rotation,
        scale);

    m_game_components.push_back(std::move(game_component));

    for (int i = 0; i < root.mNumChildren; ++i) {
        ImportChildNodes(*root.mChildren[i], scene, glm::vec3(0), glm::vec3(0), glm::vec3(1));
    }
}

ImportableInheritanceMesh::ImportableInheritanceMesh(
    Game& game,
    const std::filesystem::path& path,
    const glm::vec3& position,
    const glm::vec3& rotation,
    const glm::vec3& scale)
    : m_game(game)
{
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(path.string().c_str(), aiProcessPreset_TargetRealtime_MaxQuality);

    for (int i = 0; i < scene->mNumMaterials; ++i) {
        aiString name = scene->mMaterials[i]->GetName();
        for (int j = 0; j < scene->mMaterials[i]->GetTextureCount(aiTextureType_DIFFUSE); ++j) {
            aiString str;
            scene->mMaterials[i]->GetTexture(aiTextureType_DIFFUSE, 0, &str);
            std::cout << str.C_Str();
        }
    }

    ImportChildNodes(*scene->mRootNode, *scene, position, rotation, scale);

    FillMeshes(*scene->mRootNode, *scene, 0, -1);
}

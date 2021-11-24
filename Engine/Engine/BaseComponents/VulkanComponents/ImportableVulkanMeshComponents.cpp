#include "ImportableVulkanMeshComponents.h"

#include "VulkanMeshComponent.h"
#include "../LitMaterial.h"
#include "../UnlitMaterial.h"
#include "../Relation.h"
#include "../MeshComponent.h"
#include "../TransformComponent.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <memory>

namespace diffusion {

namespace {

void add_material(
    int mat_index,
    const aiScene& scene,
    ::entt::registry& registry,
    ::entt::entity parent_entity)
{
    aiString name = scene.mMaterials[mat_index]->GetName();
    aiString str;
    for (int tex_index = 0; tex_index < scene.mMaterials[mat_index]->GetTextureCount(aiTextureType_DIFFUSE); ++tex_index) {
        scene.mMaterials[mat_index]->GetTexture(aiTextureType_DIFFUSE, tex_index, &str);
    }

    aiString normal_str;
    for (int tex_index = 0; tex_index < scene.mMaterials[mat_index]->GetTextureCount(aiTextureType_NORMALS); ++tex_index) {
        scene.mMaterials[mat_index]->GetTexture(aiTextureType_NORMALS, tex_index, &normal_str);
    }
    /*
    aiString other_str;
    for (int k = 0; k < aiTextureType_UNKNOWN; ++k) {
        for (int j = 0; j < scene.mMaterials[i]->GetTextureCount(aiTextureType(k)); ++j) {
            scene.mMaterials[i]->GetTexture(aiTextureType(k), 0, &other_str);
        }
    }
    */
    if (str.length == 0) {
        registry.emplace<UnlitMaterialComponent>(parent_entity, "default.png");
    }
    else {
        registry.emplace<LitMaterialComponent>(parent_entity, std::filesystem::path(str.C_Str()), normal_str.length == 0 ? "default_normal.png" : std::filesystem::path(normal_str.C_Str()));
    }
}

void fill_meshes(
    const aiNode& root,
    const aiScene& scene,
    ::entt::registry& registry,
    ::entt::entity parent_entity)
{
    for (int i = 0; i < root.mNumMeshes; ++i) {
        std::vector<PrimitiveColoredVertex> verticies;

        auto index = root.mMeshes[i];
        for (int j = 0; j < scene.mMeshes[index]->mNumVertices; ++j) {
            PrimitiveColoredVertex vertex;
            vertex.x = scene.mMeshes[index]->mVertices[j].x;
            vertex.y = scene.mMeshes[index]->mVertices[j].y;
            vertex.z = scene.mMeshes[index]->mVertices[j].z;

            vertex.tex_coords[0] = scene.mMeshes[index]->mTextureCoords[0][j].x;
            vertex.tex_coords[1] = scene.mMeshes[index]->mTextureCoords[0][j].y;

            vertex.norm_coords[0] = scene.mMeshes[index]->mNormals[j].x;
            vertex.norm_coords[1] = scene.mMeshes[index]->mNormals[j].y;
            vertex.norm_coords[2] = scene.mMeshes[index]->mNormals[j].z;
            /*
            vertex.color[0] = 0.7f;
            vertex.color[1] = 0.7f;
            vertex.color[2] = 0.7f;
            vertex.color[3] = 1.0f;
            */
            verticies.push_back(vertex);
        }

        std::vector<uint32_t> indexes;
        for (int j = 0; j < scene.mMeshes[index]->mNumFaces; ++j) {
            for (int k = 0; k < scene.mMeshes[index]->mFaces[j].mNumIndices; ++k) {
                indexes.push_back(scene.mMeshes[index]->mFaces[j].mIndices[k]);
            }
        }

        auto child_entity = registry.create();
        registry.emplace<Relation>(child_entity, parent_entity);

        glm::mat4 matrix;
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
                matrix[i][j] = root.mTransformation[j][i];
            }
        }
        registry.emplace<TransformComponent>(child_entity, matrix);
        //registry.emplace<TransformComponent>(child_entity, glm::mat4(1));

        registry.emplace<SubMesh>(child_entity, verticies, indexes);
        add_material(scene.mMeshes[index]->mMaterialIndex, scene, registry, child_entity);
    }

    for (int i = 0; i < root.mNumChildren; ++i) {
        auto child_entity = registry.create();
        registry.emplace<Relation>(child_entity, parent_entity);
        registry.emplace<TransformComponent>(child_entity, glm::mat4(1));
        fill_meshes(*root.mChildren[i], scene, registry, child_entity);
    }
}

}

void import_mesh(const std::filesystem::path& path, ::entt::registry& registry, ::entt::entity parent_entity)
{
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path.string().c_str(), aiProcessPreset_TargetRealtime_MaxQuality | aiProcess_FlipUVs);

    fill_meshes(*scene->mRootNode, *scene, registry, parent_entity);
}

} // namespace diffusion {
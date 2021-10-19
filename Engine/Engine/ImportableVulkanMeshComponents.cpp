#include "ImportableVulkanMeshComponents.h"

#include "VulkanMeshComponentManager.h"
#include "Material.h"
#include "ComponentGuard.h"
#include "VulkanMeshComponent.h"
#include "Entity.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <memory>

namespace diffusion {

ImportableVulkanMeshComponents::ImportableVulkanMeshComponents(
	Game& game,
	const std::filesystem::path& path,
	const std::vector<Tag>& tags,
	Entity* parent)
	: Component(concat_vectors({ s_importable_vulkan_mesh_component_tag }, tags), parent), m_game(game)
{
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path.string().c_str(), aiProcessPreset_TargetRealtime_MaxQuality | aiProcess_FlipUVs);

    m_start_material_index = IMaterial::get_start_id();

    for (int i = 0; i < scene->mNumMaterials; ++i) {
        aiString name = scene->mMaterials[i]->GetName();
        aiString str;
        for (int j = 0; j < scene->mMaterials[i]->GetTextureCount(aiTextureType_DIFFUSE); ++j) {
            scene->mMaterials[i]->GetTexture(aiTextureType_DIFFUSE, 0, &str);
        }

        aiString normal_str;
        for (int j = 0; j < scene->mMaterials[i]->GetTextureCount(aiTextureType_NORMALS); ++j) {
            scene->mMaterials[i]->GetTexture(aiTextureType_NORMALS, 0, &normal_str);
        }
        /*
        aiString other_str;
        for (int k = 0; k < aiTextureType_UNKNOWN; ++k) {
            for (int j = 0; j < scene->mMaterials[i]->GetTextureCount(aiTextureType(k)); ++j) {
                scene->mMaterials[i]->GetTexture(aiTextureType(k), 0, &other_str);
            }
        }
        */
        IMaterial* material;
        if (str.length == 0) {
            material = new DefaultMaterial(game);
        }
        else {
            material = new ImportableMaterial(game, std::filesystem::path(str.C_Str()), std::filesystem::path(normal_str.C_Str()));
        }

        s_vulkan_mesh_component_manager.register_material(MaterialType::Opaque, material);
    }

    FillMeshes(*scene->mRootNode, *scene, 0, -1);
}

int ImportableVulkanMeshComponents::FillMeshes(
    const aiNode& root,
    const aiScene& scene,
    int index,
    int parent_index)
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

        Component & comp = get_parrent()->add_component(ComponentGuard(std::make_unique<VulkanMeshComponent>(m_game, verticies, indexes, std::vector<Tag>{}, get_parrent())));
        s_vulkan_mesh_component_manager.register_mesh(m_start_material_index + scene.mMeshes[index]->mMaterialIndex, static_cast<MeshComponent*>(&comp));
    }

    int own_index = index;
    for (int i = 0; i < root.mNumChildren; ++i) {
        index = FillMeshes(*root.mChildren[i], scene, index + 1, own_index);
    }

    return index;
}

} // namespace diffusion {
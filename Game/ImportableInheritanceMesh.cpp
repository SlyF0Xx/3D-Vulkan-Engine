#include "ImportableInheritanceMesh.h"

#include "PrimitiveMesh.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

ImportableInheritanceMesh::ImportableInheritanceMesh(
    Game& game,
    const std::filesystem::path& path,
    const glm::vec3& position,
    const glm::vec3& rotation,
    const glm::vec3& scale,
    const glm::mat4& CameraMatrix,
    const glm::mat4& ProjectionMatrix)
{
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(path.string().c_str(), aiProcessPreset_TargetRealtime_MaxQuality);

	for (int i = 0; i < scene->mNumMeshes; ++i) {
        std::vector<PrimitiveColoredVertex> verticies;

        for (int j = 0; j < scene->mMeshes[i]->mNumVertices; ++j) {
            PrimitiveColoredVertex vertex;
            vertex.x = scene->mMeshes[i]->mVertices[j].x;
            vertex.y = scene->mMeshes[i]->mVertices[j].y;
            vertex.z = scene->mMeshes[i]->mVertices[j].z;
            vertex.color[0] = 0.7f;
            vertex.color[1] = 0.7f;
            vertex.color[2] = 0.7f;
            vertex.color[3] = 1.0f;
            verticies.push_back(vertex);
        }

        std::vector<uint32_t> indexes;
        for (int j = 0; j < scene->mMeshes[i]->mNumFaces; ++j) {
            for (int k = 0; k < scene->mMeshes[i]->mFaces[j].mNumIndices; ++k) {
                indexes.push_back(scene->mMeshes[i]->mFaces[j].mIndices[k]);
            }
        }

        // TODO: generate it!
        BoundingSphere bounding_sphere;
        game.register_mesh(0, new PrimitiveMesh(game, verticies, indexes, bounding_sphere, position, rotation, scale, CameraMatrix, ProjectionMatrix));
	}
}

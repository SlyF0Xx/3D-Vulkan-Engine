#include <Engine.h>

#include "MainLayout.h"
#include "MainWindow.h"

#include "Entities/ImportableEntity.h"
#include "Entities/CubeEntity.h"
#include "Entities/PlaneEntity.h"
#include "BaseComponents/CameraComponent.h"
#include "BaseComponents/PossessedComponent.h"
#include "Archiver.h"
#include "BaseComponents/Relation.h"
#include "BaseComponents/LitMaterial.h"
#include "BaseComponents/UnlitMaterial.h"
#include "BaseComponents/MeshComponent.h"
#include "Entities/DirectionalLightEntity.h"
#include "BaseComponents/DirectionalLightComponent.h"
#include "BaseComponents/PointLightComponent.h"
#include "BaseComponents/TagComponent.h"
#include "BaseComponents/DebugComponent.h"
#include "Entities/DebugCube.h"

#include <Engine.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_decompose.hpp>

#include <chrono>
#include <iostream>
#include <map>
#include <fstream>

void import_scene(Game& vulkan) {
	std::ifstream fin("sample_scene.json");
	std::string str {std::istreambuf_iterator<char>(fin),
					 std::istreambuf_iterator<char>()};

	NJSONInputArchive json_in(str);
	entt::basic_snapshot_loader loader(vulkan.get_registry());
	loader.entities(json_in)
		.component<diffusion::BoundingComponent, diffusion::CameraComponent, diffusion::SubMesh, diffusion::PossessedEntity,
				   diffusion::Relation, diffusion::LitMaterialComponent, diffusion::UnlitMaterialComponent, diffusion::TransformComponent,
				   diffusion::MainCameraTag, diffusion::DirectionalLightComponent, diffusion::TagComponent, diffusion::PointLightComponent,
				   diffusion::debug_tag /* should be ignored in runtime*/>(json_in);

	auto main_entity = vulkan.get_registry().view<diffusion::PossessedEntity>().front();
	vulkan.get_registry().set<diffusion::PossessedEntity>(main_entity);
	vulkan.get_registry().set<diffusion::MainCameraTag>(main_entity);

	/*
    vulkan.get_registry().view<diffusion::SubMesh>(entt::exclude<diffusion::debug_tag>).each([&vulkan](const diffusion::SubMesh& mesh) {
        auto parrent = entt::to_entity(vulkan.get_registry(), mesh);
        auto& transform = vulkan.get_registry().get<diffusion::TransformComponent>(parrent);

        glm::vec3 scale;
        glm::quat rotation;
        glm::vec3 translation;
        glm::vec3 skew;
        glm::vec4 perspective;
        glm::decompose(diffusion::calculate_global_world_matrix(vulkan.get_registry(), transform), scale, rotation, translation, skew, perspective);
        glm::vec3 euler = glm::eulerAngles(rotation);

        glm::vec3 delta = mesh.m_bounding_box.max - mesh.m_bounding_box.min;
        glm::vec3 delta_2 = glm::vec3(delta.x / 2, delta.y / 2, delta.z / 2);

        auto entity = diffusion::create_debug_cube_entity(
            vulkan.get_registry(),
            mesh.m_bounding_box.min + delta_2,
            glm::vec3(0),
            delta
        );
        vulkan.get_registry().emplace<diffusion::TagComponent>(entity, "debug");
        vulkan.get_registry().emplace<diffusion::Relation>(entity, parrent);
    });
	*/
}

int main() {
	diffusion::Ref<Game> vulkan = diffusion::CreateRef<Game>();

	diffusion::Ref<Editor::EditorLayout> layout =
		diffusion::CreateRef<Editor::MainLayout>(vulkan);

	diffusion::Ref <Editor::EditorWindow> container = 
		diffusion::CreateRef<Editor::MainWindow>(layout, vulkan);

	if (!container->Create()) {
		return 1;
	}

	import_scene(*vulkan);

	container->StartMainLoop();

	return 0;
}
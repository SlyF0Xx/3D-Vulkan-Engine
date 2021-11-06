#include <Engine.h>

#include "MainLayout.h"
#include "MainWindow.h"

void import_scene(Game& vulkan) {
	std::ifstream fin("sample_scene.json");
	std::string str {std::istreambuf_iterator<char>(fin),
					 std::istreambuf_iterator<char>()};

	NJSONInputArchive json_in(str);
	entt::basic_snapshot_loader loader(vulkan.get_registry());
	loader.entities(json_in)
		.component<diffusion::BoundingComponent, diffusion::CameraComponent, diffusion::SubMesh, diffusion::PossessedEntity,
				   diffusion::Relation, diffusion::LitMaterialComponent, diffusion::UnlitMaterialComponent, diffusion::TransformComponent,
				   diffusion::MainCameraTag, diffusion::DirectionalLightComponent, diffusion::TagComponent>(json_in);

	auto main_entity = vulkan.get_registry().view<diffusion::PossessedEntity>().front();
	vulkan.get_registry().set<diffusion::PossessedEntity>(main_entity);
	vulkan.get_registry().set<diffusion::MainCameraTag>(main_entity);
}

int main() {
	diffusion::Ref <Editor::EditorWindow> container = diffusion::CreateRef<Editor::MainWindow>();

	diffusion::Ref<Game> vulkan = diffusion::CreateRef<Game>();

	diffusion::Ref<Editor::EditorLayout> layout =
		diffusion::CreateRef<Editor::MainLayout>(vulkan, container);

	container->SetContext(vulkan);
	container->SetLayout(layout);

	if (!container->Create()) {
		return 1;
	}

	import_scene(*vulkan);

	container->StartMainLoop();

	return 0;
}
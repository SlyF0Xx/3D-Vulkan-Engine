#include "Scene.h"

Editor::Scene::Scene(uint32_t id) {
	m_ID = id;

	// m_Context = diffusion::CreateRef<Game>();
	m_Context = new Game();
}

void Editor::Scene::FillBasic() {
	auto mainEntity = m_Context->get_registry().create();
	m_Context->get_registry().set<diffusion::PossessedEntity>(mainEntity);
	m_Context->get_registry().emplace<diffusion::TagComponent>(mainEntity, "Main Entity");

	auto mainCamera = m_Context->get_registry().create();
	m_Context->get_registry().emplace<diffusion::TransformComponent>(mainCamera, diffusion::create_matrix(glm::vec3(0, -10, 5)));
	m_Context->get_registry().emplace<diffusion::CameraComponent>(mainCamera);
	m_Context->get_registry().set<diffusion::MainCameraTag>(mainCamera);
	m_Context->get_registry().emplace<diffusion::TagComponent>(mainCamera, "Main Camera");

	m_Context->get_registry().emplace<diffusion::PossessedEntity>(mainEntity, mainEntity);
	m_Context->get_registry().emplace<diffusion::MainCameraTag>(mainCamera, mainCamera);

	/*auto directionalLight = diffusion::create_directional_light_entity(m_Context->get_registry(), glm::vec3(4.0f, -4.0f, -3.0f), glm::vec3(glm::pi<float>() / 2, 0.0f, 0.0f));
	m_Context->get_registry().emplace<diffusion::TagComponent>(directionalLight, "Directional light");*/
	auto pointLight = diffusion::create_point_light_entity(m_Context->get_registry(), glm::vec3(0.0f, -4.0f, -1.0f));
	m_Context->get_registry().emplace<diffusion::TagComponent>(pointLight, "Point light");
}

void Editor::Scene::SetData(nlohmann::json& data) {
	m_SceneData = data;

	m_Title = data["Title"];
	m_ID = data["ID"];

	m_HasData = true;
}

void Editor::Scene::Save(std::filesystem::path& source) {
	m_Context->save_scene(source / GetFileName());
}

void Editor::Scene::Load(std::filesystem::path& source) {
	if (!m_IsEmpty) {
		return;
	}

	if (m_HasData) {
		std::string fileName = m_SceneData["FileName"];
		m_Context->load_scene(source / fileName);
	} else {
		FillBasic();
	}
	m_IsEmpty = false;
}

std::string Editor::Scene::GetFileName() const {
	return "Source_" + GetTitle() + "_" + std::to_string(GetID()) + ".json";
}

const std::string Editor::Scene::GetTitle() const {
	return m_Title;
}

//const diffusion::Ref<Game> Editor::Scene::GetContext() const {
//    return m_Context;
//}

Game* Editor::Scene::GetContext() const {
	return m_Context;
}

const uint32_t Editor::Scene::GetID() const {
	return m_ID;
}

void Editor::Scene::CreateEditorCamera() {
	// TODO: Hide it from scene hierarchy.
	m_EditorCamera = m_Context->get_registry().create();
	m_Context->get_registry().emplace<diffusion::TransformComponent>(m_EditorCamera, diffusion::create_matrix());
	m_Context->get_registry().emplace<diffusion::CameraComponent>(m_EditorCamera);
	m_Context->get_registry().emplace<diffusion::debug_tag>(m_EditorCamera);
	m_Context->get_registry().set<diffusion::MainCameraTag>(m_EditorCamera);
	m_Context->get_registry().emplace<diffusion::TagComponent>(m_EditorCamera, "Editor Camera");
}
